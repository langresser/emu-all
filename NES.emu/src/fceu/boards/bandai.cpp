/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2007 CaH4e3
 *  Copyright (C) 2011 FCEUX team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Bandai mappers
 *
 */

//Famicom Jump 2 should get transformed to m153
//All other games are not supporting EEPROM saving right now.
//We may need to distinguish between 16 and 159 in order to know the EEPROM configuration.
//Until then, we just return 0x00 from the EEPROM read

#include "mapinc.h"

namespace BoardBandai
{

static uint8 reg[16], is153;
static uint8 IRQa;
static int16 IRQCount, IRQLatch;

static uint8 *WRAM=NULL;
static uint32 WRAMSIZE;

static SFORMAT StateRegs[]=
{
  {reg, 16, "REGS"},
  {&IRQa, 1, "IRQA"},
  {&IRQCount, 2, "IRQC"},
  {&IRQLatch, 2, "IRQL"}, // need for Famicom Jump II - Saikyou no 7 Nin (J) [!]
  {0}
};

static void BandaiIRQHook(int a)
{
  if(IRQa)
  {
    IRQCount -= a;
    if(IRQCount<0)
    {
      X6502_IRQBegin(FCEU_IQEXT);
      IRQa = 0;
      IRQCount = -1;
    }
  }
}

static void BandaiSync(void)
{
  if(is153)
  {
    int base=(reg[0]&1)<<4;
    setchr8(0);
    setprg16(0x8000,(reg[8]&0x0F)|base);
    setprg16(0xC000,0x0F|base);
  }
  else
  {
    int i;
    for(i=0; i<8; i++) setchr1(i<<10,reg[i]);
    setprg16(0x8000,reg[8]);
    setprg16(0xC000,~0);
  }
  switch(reg[9]&3)
  {
    case 0: setmirror(MI_V); break;
    case 1: setmirror(MI_H); break;
    case 2: setmirror(MI_0); break;
    case 3: setmirror(MI_1); break;
  }
}

static DECLFW(BandaiWrite)
{
  A&=0x0F;
  if(A<0x0A)
  {
    reg[A&0x0F]=V;
    BandaiSync();
  }
  else
    switch(A)
    {
      case 0x0A: X6502_IRQEnd(FCEU_IQEXT); IRQa=V&1; IRQCount=IRQLatch; break;
      case 0x0B: IRQLatch&=0xFF00; IRQLatch|=V;  break;
      case 0x0C: IRQLatch&=0xFF; IRQLatch|=V<<8; break;
      case 0x0D: break;// Serial EEPROM control port
    }
}

DECLFR(BandaiRead)
{
	return 0;
}

static void BandaiPower(void)
{
  BandaiSync();
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x6000,0xFFFF,BandaiWrite);
  SetReadHandler(0x6000,0x7FFF,BandaiRead);
}

static void M153Power(void)
{
  BandaiSync();
  setprg8r(0x10,0x6000,0);
  SetReadHandler(0x6000,0x7FFF,CartBR);
  SetWriteHandler(0x6000,0x7FFF,CartBW);
  SetReadHandler(0x8000,0xFFFF,CartBR);
  SetWriteHandler(0x8000,0xFFFF,BandaiWrite);
}


static void M153Close(void)
{
  if(WRAM)
    FCEU_gfree(WRAM);
  WRAM=NULL;
}

static void StateRestore(int version)
{
  BandaiSync();
}

}

void Mapper16_Init(CartInfo *info)
{
	using namespace BoardBandai;
  is153=0;
  info->Power=BandaiPower;
  MapIRQHook=BandaiIRQHook;
  GameStateRestore=BoardBandai::StateRestore;
  AddExState(&BoardBandai::StateRegs, ~0, 0, 0);
}

void Mapper159_Init(CartInfo *info)
{
  Mapper16_Init(info);
}

void Mapper153_Init(CartInfo *info)
{
	using namespace BoardBandai;
  is153=1;
  info->Power=M153Power;
  info->Close=M153Close;
  MapIRQHook=BandaiIRQHook;

  WRAMSIZE=8192;
  BoardBandai::WRAM=(uint8*)FCEU_gmalloc(WRAMSIZE);
  SetupCartPRGMapping(0x10,BoardBandai::WRAM,WRAMSIZE,1);
  AddExState(BoardBandai::WRAM, WRAMSIZE, 0, "WRAM");

  if(info->battery)
  {
    info->SaveGame[0]=BoardBandai::WRAM;
    info->SaveGameLen[0]=WRAMSIZE;
  }

  GameStateRestore=BoardBandai::StateRestore;
  AddExState(&BoardBandai::StateRegs, ~0, 0, 0);
}
