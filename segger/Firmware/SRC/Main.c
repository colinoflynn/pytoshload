/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*           (C) 2000    SEGGER Microcontroller Systeme GmbH          *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : Main.c
Purpose : General RAM image communication routines
---------------------------END-OF-HEADER------------------------------
*/

#include "Flash.h"
#include "System.h"
#include "Config.h"

// shared modules
#include "CRC.h"
#include "Prot.h"
#include "TargetCMD.h"

/*********************************************************************
*
*       Defines (main)
*
**********************************************************************
*/

// Default values
#ifndef DISABLE_GET_CRC
  #define DISABLE_GET_CRC       0
#endif
#ifndef DISABLE_ERASE_SECTOR
  #define DISABLE_ERASE_SECTOR  0
#endif
#ifndef DISABLE_PROTECT_BLOCK
  #define DISABLE_PROTECT_BLOCK 0
#endif
#ifndef DISABLE_BLANK_CHECK
  #define DISABLE_BLANK_CHECK   0
#endif
#ifndef DISABLE_GET_SUM
  #define DISABLE_GET_SUM       1
#endif

#define SD1  0xEB
#define SD2  0xED
#define ED   0xEC

#define STATE_SD1      1
#define STATE_SD2      2
#define STATE_LEN1     3
#define STATE_LEN2     4
#define STATE_DATA     5
#define STATE_CHECKSUM 6
#define STATE_ED       7

#ifndef BUFFER_ADR
  #define GET_BUFFER_ADR() _aProtBuffer
#else
  #define GET_BUFFER_ADR() (U8 *)BUFFER_ADR
#endif

#ifdef REMAP_TMP86FS49
  U16 FLASH_RemapAddr(U16 Addr);
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U8   _aBuffer[7];
static U8 * _pBuffer;
static U8 * _pData;
static U32  _SAdr;
static U16  _Len;
static U16  _Crc;

static U8 * _pWrite;
static int  _RxState;
static int  _RxLen;
static U8   _RxChecksum;
static int  _RemDataBytes;

static U8 * _pDataPara;
static int  _Size, _SizePara;
#ifndef BUFFER_ADR
  static U8 _aProtBuffer[BUFFER_SIZE + 10]; /* BUFFER_SIZE bytes buffer + 10 bytes parameter */
#endif

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _WriteU16
*/
static void _WriteU16(U16 Value) {
  #ifdef RW_UNALIGNED
    *((U16 *)_pBuffer) = Value;
    _pBuffer += 2;
  #else
    *(_pBuffer + 0) = (U8)Value;
    *(_pBuffer + 1) = (U8)(Value >> 8);
    _pBuffer += 2;
  #endif
}

/*********************************************************************
*
*       _WriteU32
*/
static void _WriteU32(U32 Value) {
  #ifdef RW_UNALIGNED
    *((U32 *)_pBuffer) = Value;
    _pBuffer += 4;
  #else
    _WriteU16((U16)Value);
    _WriteU16((U16)(Value >> 16));
  #endif
}

/*********************************************************************
*
*       _ReadU16
*/
static U16 _ReadU16(void) {
  #ifdef RW_UNALIGNED
    U16 * p = (U16*)_pData;
    _pData += 2;
    return *p;
  #else
    U16 Value = *_pData;
    Value |= ((U16)(*(_pData + 1)) << 8);
    _pData += 2;
    return Value;
  #endif
}

/*********************************************************************
*
*       _ReadU32
*/
static U32 _ReadU32(void) {
  #ifdef RW_UNALIGNED
    U32 * p = (U32*)_pData;
    _pData += 4;
    return *p;
  #else
    U32 Value32 = _ReadU16();
    Value32 |= ((U32)(_ReadU16()) << 16);
    return Value32;
  #endif
}

/*********************************************************************
*
*       _ReadSAdrLen
*/
static void _ReadSAdrLen(void) {
  _SAdr = _ReadU32();
  _Len  = _ReadU16();
}
/*********************************************************************
*
*       _Ack
*/
static void _Ack(void) {
  _aBuffer[0] = CMD_ACK;
  PROT_Send(1);
}

/*********************************************************************
*
*       _Error
*/
static void _Error(U32 Adr, U8 Code) {
  _aBuffer[0] = CMD_ERROR;    // U8  cmd
  _aBuffer[1] = Code;         // U8  code
  _pBuffer = &_aBuffer[2];
  _WriteU32(Adr);             // U32 adr
  #ifdef REMAP_TMP86FS49
    *_pBuffer = *((U8 *)FLASH_RemapAddr((U16)Adr));
  #else
    *_pBuffer = *((U8 *)Adr);   // U8  contents
  #endif
  PROT_Send(7);
}

/*********************************************************************
*
*       _OnCmdId
*/
static void _OnCmdId(void) {
  _aBuffer[0] = CMD_ID;                        // U8  cmd
  _pBuffer = &_aBuffer[1];
  _WriteU32(0xf);                              // U32 flags
  _WriteU16((U16)(BUFFER_SIZE));               // U16 max databytes
  PROT_Send(7);
}

/*********************************************************************
*
*       _OnCmdEraseSector
*/
#if !DISABLE_ERASE_SECTOR
static void _OnCmdEraseSector(void) {
  _SAdr = _ReadU32();
  if (FLASH_EraseBlock(_SAdr)) {
    _Error(_SAdr, ERROR_ERASE);
  } else {
    _Ack();
  }
}
#endif

/*********************************************************************
*
*       _OnCmdProtectBlock
*/
#if !DISABLE_PROTECT_BLOCK
static void _OnCmdProtectBlock(void) {
  _SAdr = _ReadU32();
  FLASH_ProtectBlock(_SAdr);
  _Ack();
}
#endif

/*********************************************************************
*
*       _OnCmdReadBack
*/
static void _OnCmdReadBack(void) {
  _ReadSAdrLen();
  _aBuffer[0] = CMD_READ_BACK; // U8  cmd
  _pBuffer = &_aBuffer[1];
  _WriteU32(_SAdr);            // U32 adr
  _WriteU16(_Len);             // U16 len
  PROT_SendData(7, _Len);
}

/*********************************************************************
*
*       _OnCmdProgram
*/
static void _OnCmdProgram(void) {
  U8 * p;
  _ReadSAdrLen();
  p = (U8 *)_SAdr;
  if (FLASH_Write(_SAdr, _pData, _Len)) {
    _Error(_SAdr - 1, ERROR_WRITE);
    return;
  }
  while(_Len--) {
    #ifdef REMAP_TMP86FS49
      U8 * pMap = (U8 *)FLASH_RemapAddr((U16)p++);
      if (*(pMap) != *(_pData++)) {
        _Error((U32)_pData - 1, ERROR_COMPARE);
        return;
      }
    #else
      if (*p++ != *_pData++) {
        _Error(_SAdr - 1, ERROR_COMPARE);
        return;
      }
    #endif
  }
  _Ack();
}

/*********************************************************************
*
*       _OnCmdGetCRC
*/
#if !DISABLE_GET_CRC
static void _OnCmdGetCRC(void) {
  U8  Init;
  U32 Len32;
  _SAdr = _ReadU32();
  Len32 = _ReadU32();
  Init = *_pData;
  if (Init) {
    CRC_Init(&_Crc);
  }
  CRC_Calc(&_Crc, (U8 *)_SAdr, Len32);
  _aBuffer[0] = CMD_GET_CRC;      // U8  cmd
  _pBuffer = &_aBuffer[1];
  _WriteU16(_Crc);                // U16 crc
  PROT_Send(3);
}
#endif

/*********************************************************************
*
*       _OnCmdGetSUM
*/
#if !DISABLE_GET_SUM
static void _OnCmdGetSUM(void) {
  U8 * pMem;
  _SAdr = _ReadU32();
  pMem = (U8 *)_SAdr;
  _SAdr = _ReadU32() + 1;         // Use _SAdr for length to save memory
  if (*_pData) {
    _Crc  = 0;
  }
  while (_SAdr--) {
    #ifdef REMAP_TMP86FS49
      U8 * pData = (U8 *)FLASH_RemapAddr((U16)pMem++);
      _Crc += *(pData);
    #else
      _Crc += *(pMem++);
    #endif
  }
  _aBuffer[0] = CMD_GET_SUM;      // U8  cmd
  _pBuffer = &_aBuffer[1];
  _WriteU16(_Crc);                // U16 sum
  PROT_Send(3);
}
#endif

/*********************************************************************
*
*       _OnCmdBlankCheck
*/
#if !DISABLE_BLANK_CHECK
static void _OnCmdBlankCheck(void) {
  _ReadSAdrLen();
  while(_Len--) {
    #ifdef REMAP_TMP86FS49
      U8 * pData = (U8 *)FLASH_RemapAddr((U16)_SAdr++);
      if ((*pData) != 0xff) {
        _Error(_SAdr - 1, ERROR_BLANKCHECK);
        return;
      }
    #else
      if (*((U8 *)_SAdr++) != 0xff) {
        _Error(_SAdr - 1, ERROR_BLANKCHECK);
        return;
      }
    #endif
  }
  _Ack();
}
#endif

/*********************************************************************
*
*       _Notify
*/
static void _Notify(void) {
  _pData = GET_BUFFER_ADR();
  switch (*_pData++) {
    case CMD_ID:
      _OnCmdId();
      break;
    #if !DISABLE_ERASE_SECTOR
      case CMD_ERASE_SECTOR:
        _OnCmdEraseSector();
        break;
    #endif
    case CMD_READ_BACK:
      _OnCmdReadBack();
      break;
    case CMD_PROGRAM:
      _OnCmdProgram();
      break;
    #if !DISABLE_GET_CRC
      case CMD_GET_CRC:
        _OnCmdGetCRC();
        break;
    #endif
    #if !DISABLE_GET_SUM
      case CMD_GET_SUM:
        _OnCmdGetSUM();
        break;
    #endif
    #if !DISABLE_BLANK_CHECK
      case CMD_BLANK_CHECK:
        _OnCmdBlankCheck();
        break;
    #endif
    #if !DISABLE_PROTECT_BLOCK
      case CMD_PROTECT_BLOCK:
        _OnCmdProtectBlock();
        break;
    #endif
  }
}

/*********************************************************************
*
*       _SendPacket
*/
static void _SendPacket(void) {
  U8 Len1     = (U8) (_Size + _SizePara);
  U8 Len2     = (U8)((_Size + _SizePara) >> 8);
  U8 Checksum = Len1 + Len2;
  SYSTEM_SIO_Send1(SD1);
  SYSTEM_SIO_Send1(SD2);
  SYSTEM_SIO_Send1(Len1);
  SYSTEM_SIO_Send1(Len2);
  while(_Size--) {
    Checksum  += *_pData;
    SYSTEM_SIO_Send1(*_pData++);
  }
  while(_SizePara--) {
    #ifdef REMAP_TMP86FS49
      U8 * pData = (U8 *)FLASH_RemapAddr((U16)_pDataPara);
      Checksum  += *pData;
      SYSTEM_SIO_Send1(*pData);
      _pDataPara++;
    #else
      Checksum  += *_pDataPara;
      SYSTEM_SIO_Send1(*_pDataPara++);
    #endif
  }
  SYSTEM_SIO_Send1(Checksum);
  SYSTEM_SIO_Send1(ED);
}

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       main
*/
void main(void) {
  _RxState       = STATE_SD1;
  while(1) {
    U8 Byte = SYSTEM_SIO_Get1();
    PROT_OnRx(Byte);
  }
}

/*********************************************************************
*
*       PROT_Send
*/
void PROT_Send(int Size) {
  _pData     = _aBuffer;
  _Size      = Size;
  _SizePara  = 0;
  _SendPacket();
}

/*********************************************************************
*
*       PROT_SendData
*/
void PROT_SendData(int Size, int SizePara) {
  _pData     = _aBuffer;
  _pDataPara = (U8 *)_SAdr;
  _Size      = Size;
  _SizePara  = SizePara;
  _SendPacket();
}

/*********************************************************************
*
*       PROT_OnRx
*/
void PROT_OnRx(U8 Byte) {
  switch (_RxState) {
    case STATE_SD1:
      if (Byte != SD1)
        return;
      _RxState = STATE_SD2;
      break;
    case STATE_SD2:
      if (Byte != SD2) {
        _RxState = STATE_SD1;
        return;
      }
      _RxState = STATE_LEN1;
      break;
    case STATE_LEN1:
      _RxChecksum = Byte;
      _RxLen = Byte;
      _RxState = STATE_LEN2;
      break;
    case STATE_LEN2:
      _RxChecksum += Byte;
      _RxLen |= ((U16)Byte << 8);
      if (_RxLen) {
        _RxState      = STATE_DATA;
        _pWrite       = GET_BUFFER_ADR();
        _RemDataBytes = _RxLen;
      } else {
        _RxState = STATE_CHECKSUM;
      }
      break;
    case STATE_DATA:
      _RxChecksum += Byte;
      if (_RemDataBytes--) {
        *_pWrite++ = Byte;
      }
      if (!_RemDataBytes) {
        _RxState = STATE_CHECKSUM;
      }
      break;
    case STATE_CHECKSUM:
      if (Byte != _RxChecksum) {
        _aBuffer[0] = CMD_NACK;
        PROT_Send(1);
        _RxState = STATE_SD1;
      } else {
        _RxState = STATE_ED;
      }
      break;
    case STATE_ED:
      _RxState = STATE_SD1;
      _Notify();
      break;
  }
}

