// Copyright 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QTableWidget>

#include "Common/CommonTypes.h"

namespace AddressSpace
{
enum class Type;
}

class MemoryViewWidget : public QTableWidget
{
  Q_OBJECT
public:
  enum class Type : int
  {
    Hex8 = 1,
    Hex16,
    Hex32,
    Unsigned8,
    Unsigned16,
    Unsigned32,
    Signed8,
    Signed16,
    Signed32,
    ASCII,
    Float32,
    Double
  };

  enum class BPType
  {
    ReadWrite,
    ReadOnly,
    WriteOnly
  };

  explicit MemoryViewWidget(QWidget* parent = nullptr);

  void Update();
  void UpdateFont();
  void ToggleBreakpoint();
  void ToggleRowBreakpoint(bool row);

  void SetAddressSpace(AddressSpace::Type address_space);
  AddressSpace::Type GetAddressSpace() const;
  void SetDisplay(Type type, int bytes_per_row, int alignment);
  void SetBPType(BPType type);
  void SetAddress(u32 address);

  void SetBPLoggingEnabled(bool enabled);

  u32 GetContextAddress() const;

  void resizeEvent(QResizeEvent*) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

signals:
  void BreakpointsChanged();
  void ShowCode(u32 address);
  void RequestWatch(QString name, u32 address);

private:
  void OnContextMenu();
  void OnCopyAddress();
  void OnCopyHex();

  AddressSpace::Type m_address_space{};
  Type m_type = Type::Hex8;
  BPType m_bp_type = BPType::ReadWrite;
  bool m_do_log = true;
  u32 m_context_address;
  u32 m_base_address;
  u32 m_address = 0;
  int m_font_width = 0;
  int m_font_vspace = 0;
  int m_bytes_per_row = 16;
  int m_alignment = 16;
};
