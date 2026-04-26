#include "Window.h"

#include "Array.h"
#include "Keyboard.h"
#include "Matrix.h"
#include "Mouse.h"
#include "Pointer.h"
#include "Renderer.h"
#include "String.h"
#include "Texture2D.h"
#include "Viewport.h"

#include "SFML/Graphics.hpp"

#include <cstdint>
#include <vector>

namespace {
  std::vector<Window>& GetStack() {
    static std::vector<Window> stack;
    return stack;
  }

  void ProcessMouseEvent(sf::Mouse::Button button, bool pressed) {
    switch (button) {
      case sf::Mouse::Button::Left:
        Mouse_SetPressed(MouseButton_Left, pressed); break;
      case sf::Mouse::Button::Right:
        Mouse_SetPressed(MouseButton_Right, pressed); break;
      case sf::Mouse::Button::Middle:
        Mouse_SetPressed(MouseButton_Middle, pressed); break;
      case sf::Mouse::Button::Extra1:
        Mouse_SetPressed(MouseButton_X1, pressed); break;
      case sf::Mouse::Button::Extra2:
        Mouse_SetPressed(MouseButton_X2, pressed); break;
      default: break;
    }
  }

  struct WindowImpl : public WindowT {
    sf::RenderWindow impl;
    String title;
    Viewport viewport;
    V2U size;
    uint bpp;
    bool captureMouse;
    bool hasFocus;

    WindowImpl(
        String const& title,
        V2U const& size,
        bool border,
        bool fullscreen) :
      title(title),
      size(size),
      bpp(32),
      captureMouse(false),
      hasFocus(true)
    {
      viewport = Viewport_Create(0, size, 1, true);
      impl.create(
        sf::VideoMode(sf::Vector2u(size.x, size.y), bpp),
        title,
        border ? sf::Style::Default : sf::Style::None,
        fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
      impl.setMouseCursorVisible(false);
      impl.setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f((float)size.x, (float)size.y))));
      viewport->size = size;

      // sf::Vector2i p = sf::Mouse::getPosition(impl);
      // sf::Mouse::setPosition(p, impl);
    }

    void Close() {
      impl.close();
    }

    void Display() {
      impl.display();
    }

    void* GetImplData() {
      return &impl;
    }

    V2U GetSize() const {
      return size;
    }

    bool HasFocus() const {
      return hasFocus;
    }

    bool IsOpen() const {
      return impl.isOpen();
    }

    void SetCaptureMouse(bool captureMouse) {
      this->captureMouse = captureMouse;
    }

    void SetCursorVisible(bool visible) {
      impl.setMouseCursorVisible(visible);
    }

    void SetFullscreen() {
      impl.create(
        sf::VideoMode(sf::Vector2u(size.x, size.y), bpp),
        title,
        sf::State::Fullscreen);
      viewport->size.x = (float)impl.getSize().x;
      viewport->size.y = (float)impl.getSize().y;
      impl.setMouseCursorVisible(false);
    }

    void SetIcon(Texture2D const& icon) {
      Array<uchar> buf(icon->GetMemory());
      icon->GetData(buf.data());
      impl.setIcon(sf::Image(
        sf::Vector2u(icon->GetWidth(), icon->GetHeight()),
        (std::uint8_t const*)buf.data()));
    }

    void SetPosition(V2I const& p) {
      impl.setPosition(sf::Vector2i(p.x, p.y));
    }

    void SetSync(bool sync) {
      impl.setVerticalSyncEnabled(sync);
    }

    void Update() {
      while (std::optional<sf::Event> e = impl.pollEvent()) {
        if (sf::Event::Resized const* resized = e->getIf<sf::Event::Resized>()) {
          float w = (float)resized->size.x;
          float h = (float)resized->size.y;
          impl.setView(sf::View(sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(w, h))));
          size.x = resized->size.x;
          size.y = resized->size.y;
          viewport->size = V2(w, h);
        }

        else if (sf::Event::KeyPressed const* key = e->getIf<sf::Event::KeyPressed>()) {
          if (key->code != sf::Keyboard::Key::Unknown)
            Keyboard_AddDown((int)key->code);
        }

        else if (sf::Event::MouseButtonPressed const* button = e->getIf<sf::Event::MouseButtonPressed>())
          ProcessMouseEvent(button->button, true);

        else if (sf::Event::MouseButtonReleased const* button = e->getIf<sf::Event::MouseButtonReleased>())
          ProcessMouseEvent(button->button, false);

        else if (sf::Event::MouseMoved const* mouseMoved = e->getIf<sf::Event::MouseMoved>()) {
          V2I p(mouseMoved->position.x, mouseMoved->position.y);

          if (captureMouse) {
            const V2I borderSize = 1;
            V2I s = (V2I)size - borderSize;
            if (p.x < borderSize.x ||
                p.y < borderSize.y ||
                p.x > s.x ||
                p.y > s.y)
            {
              p = Clamp(p, borderSize, s);
              Mouse_SetPos(p);
            }
          }
          Mouse_UpdatePos(p);  
        }

        else if (sf::Event::MouseWheelScrolled const* wheel = e->getIf<sf::Event::MouseWheelScrolled>()) {
          if (!hasFocus)
            continue;
          /* TODO : Improve precision on Windows. */
          Mouse_SetScrollDelta(wheel->delta);
        }

        else if (e->is<sf::Event::FocusGained>())
          hasFocus = true;

        else if (e->is<sf::Event::FocusLost>())
          hasFocus = false;

        else if (sf::Event::TextEntered const* text = e->getIf<sf::Event::TextEntered>()) {
          if (text->unicode >= 32 && text->unicode <= 126)
            Keyboard_AddText((char)text->unicode);
        }
      }
    }
  };
}

Window Window_Create(
  String const& title,
  V2U const& size,
  bool border,
  bool fullscreen)
{
  return new WindowImpl(title, size, border, fullscreen);
}

Window Window_Get() {
  return GetStack().size() ? GetStack().back() : nullptr;
}

void Window_Pop() {
  Viewport_Pop();
  GetStack().pop_back();
}

void Window_Push(Window const& window) {
  GetStack().push_back(window);
  Viewport_Push(((WindowImpl*)window.get())->viewport);
}
