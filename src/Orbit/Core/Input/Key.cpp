/*
 * Copyright (c) 2019 Sebastian Kylander https://gaztin.com/
 *
 * This software is provided 'as-is', without any express or implied warranty. In no event will
 * the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the
 *    original software. If you use this software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as
 *    being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "Key.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#endif

ORB_NAMESPACE_BEGIN

Key ConvertSystemKey( uint32_t system_key )
{

#if defined( ORB_OS_WINDOWS )

	switch( system_key )
	{
		default: return Key::Unknown;

		case '0': return Key::_0;
		case '1': return Key::_1;
		case '2': return Key::_2;
		case '3': return Key::_3;
		case '4': return Key::_4;
		case '5': return Key::_5;
		case '6': return Key::_6;
		case '7': return Key::_7;
		case '8': return Key::_8;
		case '9': return Key::_9;

		case 'A': return Key::A;
		case 'B': return Key::B;
		case 'C': return Key::C;
		case 'D': return Key::D;
		case 'E': return Key::E;
		case 'F': return Key::F;
		case 'G': return Key::G;
		case 'H': return Key::H;
		case 'I': return Key::I;
		case 'J': return Key::J;
		case 'K': return Key::K;
		case 'L': return Key::L;
		case 'M': return Key::M;
		case 'N': return Key::N;
		case 'O': return Key::O;
		case 'P': return Key::P;
		case 'Q': return Key::Q;
		case 'R': return Key::R;
		case 'S': return Key::S;
		case 'T': return Key::T;
		case 'U': return Key::U;
		case 'V': return Key::V;
		case 'W': return Key::W;
		case 'X': return Key::X;
		case 'Y': return Key::Y;
		case 'Z': return Key::Z;

		case VK_F1:  return Key::F1;
		case VK_F2:  return Key::F2;
		case VK_F3:  return Key::F3;
		case VK_F4:  return Key::F4;
		case VK_F5:  return Key::F5;
		case VK_F6:  return Key::F6;
		case VK_F7:  return Key::F7;
		case VK_F8:  return Key::F8;
		case VK_F9:  return Key::F9;
		case VK_F10: return Key::F10;
		case VK_F11: return Key::F11;
		case VK_F12: return Key::F12;

		case VK_ESCAPE:   return Key::Escape;
		case VK_TAB:      return Key::Tab;
		case VK_CAPITAL:  return Key::CapsLock;
		case VK_SHIFT:    return Key::Shift;
		case VK_CONTROL:  return Key::Control;
		case VK_MENU:     return Key::Alt;
		case VK_SPACE:    return Key::Space;
		case VK_RMENU:    return Key::RightAlt;
		case VK_RCONTROL: return Key::RightControl;
		case VK_RSHIFT:   return Key::RightShift;
		case VK_RETURN:   return Key::Return;
		case VK_BACK:     return Key::Backspace;
		case VK_SNAPSHOT: return Key::PrintScreen;
		case VK_SCROLL:   return Key::ScrollLock;
		case VK_PAUSE:    return Key::Pause;
		case VK_INSERT:   return Key::Insert;
		case VK_HOME:     return Key::Home;
		case VK_PRIOR:    return Key::PageUp;
		case VK_DELETE:   return Key::Delete;
		case VK_END:      return Key::End;
		case VK_NEXT:     return Key::PageDown;
		case VK_NUMLOCK:  return Key::NumLock;
		case VK_DIVIDE:   return Key::Divide;
		case VK_MULTIPLY: return Key::Multiply;
		case VK_SUBTRACT: return Key::Subtract;
		case VK_ADD:      return Key::Add;
		case VK_DECIMAL:  return Key::DecimalPoint;
		case VK_DOWN:     return Key::ArrowDown;
		case VK_LEFT:     return Key::ArrowLeft;
		case VK_RIGHT:    return Key::ArrowRight;
		case VK_UP:       return Key::ArrowUp;
	}

#endif

}

ORB_NAMESPACE_END
