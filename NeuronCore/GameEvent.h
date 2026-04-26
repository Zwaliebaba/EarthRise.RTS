#pragma once

class GameEvent
{
  public:
  bool operator==(const GameEvent&) const = default;

  GameEvent()
    : m_consumed(false)
  {}

  void SetConsumed() { m_consumed = true; }
  bool IsConsumed() const { return m_consumed; }

  protected:
  bool m_consumed;
  virtual ~GameEvent() = default;
};
