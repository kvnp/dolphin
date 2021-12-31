// Copyright 2008 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Core/PowerPC/JitCommon/JitBase.h"

#include "Common/CommonTypes.h"
#include "Core/Config/MainSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/HW/CPU.h"
#include "Core/PowerPC/PPCAnalyst.h"
#include "Core/PowerPC/PowerPC.h"

const u8* JitBase::Dispatch(JitBase& jit)
{
  return jit.GetBlockCache()->Dispatch();
}

void JitTrampoline(JitBase& jit, u32 em_address)
{
  jit.Jit(em_address);
}

JitBase::JitBase() : m_code_buffer(code_buffer_size)
{
  m_registered_config_callback_id = Config::AddConfigChangedCallback(
      [this] { Core::RunAsCPUThread([this] { RefreshConfig(); }); });
  RefreshConfig();
}

JitBase::~JitBase()
{
  Config::RemoveConfigChangedCallback(m_registered_config_callback_id);
}

void JitBase::RefreshConfig()
{
  bJITOff = Config::Get(Config::MAIN_DEBUG_JIT_OFF);
  bJITLoadStoreOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_OFF);
  bJITLoadStorelXzOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_LXZ_OFF);
  bJITLoadStorelwzOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_LWZ_OFF);
  bJITLoadStorelbzxOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_LBZX_OFF);
  bJITLoadStoreFloatingOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_FLOATING_OFF);
  bJITLoadStorePairedOff = Config::Get(Config::MAIN_DEBUG_JIT_LOAD_STORE_PAIRED_OFF);
  bJITFloatingPointOff = Config::Get(Config::MAIN_DEBUG_JIT_FLOATING_POINT_OFF);
  bJITIntegerOff = Config::Get(Config::MAIN_DEBUG_JIT_INTEGER_OFF);
  bJITPairedOff = Config::Get(Config::MAIN_DEBUG_JIT_PAIRED_OFF);
  bJITSystemRegistersOff = Config::Get(Config::MAIN_DEBUG_JIT_SYSTEM_REGISTERS_OFF);
  bJITBranchOff = Config::Get(Config::MAIN_DEBUG_JIT_BRANCH_OFF);
  bJITRegisterCacheOff = Config::Get(Config::MAIN_DEBUG_JIT_REGISTER_CACHE_OFF);
}

bool JitBase::CanMergeNextInstructions(int count) const
{
  if (CPU::IsStepping() || js.instructionsLeft < count)
    return false;
  // Be careful: a breakpoint kills flags in between instructions
  for (int i = 1; i <= count; i++)
  {
    if (SConfig::GetInstance().bEnableDebugging &&
        PowerPC::breakpoints.IsAddressBreakPoint(js.op[i].address))
      return false;
    if (js.op[i].isBranchTarget)
      return false;
  }
  return true;
}

void JitBase::UpdateMemoryAndExceptionOptions()
{
  bool any_watchpoints = PowerPC::memchecks.HasAny();
  jo.fastmem = SConfig::GetInstance().bFastmem && jo.fastmem_arena && (MSR.DR || !any_watchpoints);
  jo.memcheck = SConfig::GetInstance().bMMU || any_watchpoints;
  jo.fp_exceptions = SConfig::GetInstance().bFloatExceptions;
  jo.div_by_zero_exceptions = SConfig::GetInstance().bDivideByZeroExceptions;
}

bool JitBase::ShouldHandleFPExceptionForInstruction(const PPCAnalyst::CodeOp* op)
{
  if (jo.fp_exceptions)
    return (op->opinfo->flags & FL_FLOAT_EXCEPTION) != 0;
  else if (jo.div_by_zero_exceptions)
    return (op->opinfo->flags & FL_FLOAT_DIV) != 0;
  else
    return false;
}
