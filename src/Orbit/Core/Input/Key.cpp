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

#include "Orbit/Core/Widget/Window.h"

#if defined( ORB_OS_WINDOWS )
#  include <Windows.h>
#elif defined( ORB_OS_LINUX )
#  include <X11/keysym.h>
#elif defined( ORB_OS_MACOS )
#  include <AppKit/AppKit.h>
#  include <Carbon/Carbon.h>
#elif defined( ORB_OS_ANDROID )
#  include <android/keycodes.h>
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

#elif defined( ORB_OS_LINUX )

	Key key = Key::Unknown;

	if( Window* window = Window::GetPtr(); window != nullptr )
	{
		int     keysyms_per_keycode_return;
		KeySym* keysym = XGetKeyboardMapping( window->GetPrivateDetails().display, system_key, 1, &keysyms_per_keycode_return );
		
		if( keysym )
		{
			switch( keysym[ 0 ] )
			{
				case XK_0: { key = Key::_0; } break;
				case XK_1: { key = Key::_1; } break;
				case XK_2: { key = Key::_2; } break;
				case XK_3: { key = Key::_3; } break;
				case XK_4: { key = Key::_4; } break;
				case XK_5: { key = Key::_5; } break;
				case XK_6: { key = Key::_6; } break;
				case XK_7: { key = Key::_7; } break;
				case XK_8: { key = Key::_8; } break;
				case XK_9: { key = Key::_9; } break;

				case XK_a: { key = Key::A; } break;
				case XK_b: { key = Key::B; } break;
				case XK_c: { key = Key::C; } break;
				case XK_d: { key = Key::D; } break;
				case XK_e: { key = Key::E; } break;
				case XK_f: { key = Key::F; } break;
				case XK_g: { key = Key::G; } break;
				case XK_h: { key = Key::H; } break;
				case XK_i: { key = Key::I; } break;
				case XK_j: { key = Key::J; } break;
				case XK_k: { key = Key::K; } break;
				case XK_l: { key = Key::L; } break;
				case XK_m: { key = Key::M; } break;
				case XK_n: { key = Key::N; } break;
				case XK_o: { key = Key::O; } break;
				case XK_p: { key = Key::P; } break;
				case XK_q: { key = Key::Q; } break;
				case XK_r: { key = Key::R; } break;
				case XK_s: { key = Key::S; } break;
				case XK_t: { key = Key::T; } break;
				case XK_u: { key = Key::U; } break;
				case XK_v: { key = Key::V; } break;
				case XK_w: { key = Key::W; } break;
				case XK_x: { key = Key::X; } break;
				case XK_y: { key = Key::Y; } break;
				case XK_z: { key = Key::Z; } break;

				case XK_F1:  { key = Key::F1;  } break;
				case XK_F2:  { key = Key::F2;  } break;
				case XK_F3:  { key = Key::F3;  } break;
				case XK_F4:  { key = Key::F4;  } break;
				case XK_F5:  { key = Key::F5;  } break;
				case XK_F6:  { key = Key::F6;  } break;
				case XK_F7:  { key = Key::F7;  } break;
				case XK_F8:  { key = Key::F8;  } break;
				case XK_F9:  { key = Key::F9;  } break;
				case XK_F10: { key = Key::F10; } break;
				case XK_F11: { key = Key::F11; } break;
				case XK_F12: { key = Key::F12; } break;

				case XK_Escape:      { key = Key::Escape;       } break;
				case XK_Tab:         { key = Key::Tab;          } break;
				case XK_Caps_Lock:   { key = Key::CapsLock;     } break;
				case XK_Shift_L:     { key = Key::Shift;        } break;
				case XK_Shift_R:     { key = Key::Shift;        } break;
				case XK_Control_L:   { key = Key::Control;      } break;
				case XK_Control_R:   { key = Key::Control;      } break;
				case XK_Alt_L:       { key = Key::Alt;          } break;
				case XK_Alt_R:       { key = Key::Alt;          } break;
				case XK_space:       { key = Key::Space;        } break;
				case XK_Return:      { key = Key::Return;       } break;
				case XK_BackSpace:   { key = Key::Backspace;    } break;
				case XK_Print:       { key = Key::PrintScreen;  } break;
				case XK_Scroll_Lock: { key = Key::ScrollLock;   } break;
				case XK_Pause:       { key = Key::Pause;        } break;
				case XK_Insert:      { key = Key::Insert;       } break;
				case XK_Home:        { key = Key::Home;         } break;
				case XK_Page_Up:     { key = Key::PageUp;       } break;
				case XK_Delete:      { key = Key::Delete;       } break;
				case XK_End:         { key = Key::End;          } break;
				case XK_Page_Down:   { key = Key::PageDown;     } break;
				case XK_Num_Lock:    { key = Key::NumLock;      } break;
				case XK_KP_Divide:   { key = Key::Divide;       } break;
				case XK_KP_Multiply: { key = Key::Multiply;     } break;
				case XK_KP_Subtract: { key = Key::Subtract;     } break;
				case XK_KP_Add:      { key = Key::Add;          } break;
				case XK_KP_Decimal:  { key = Key::DecimalPoint; } break;
				case XK_Down:        { key = Key::ArrowDown;    } break;
				case XK_Left:        { key = Key::ArrowLeft;    } break;
				case XK_Right:       { key = Key::ArrowRight;   } break;
				case XK_Up:          { key = Key::ArrowUp;      } break;
			}

			XFree( keysym );
		}
	}
	
	return key;
	
#elif defined( ORB_OS_MACOS )

	switch( system_key )
	{
		default: return Key::Unknown;

		case kVK_ANSI_0: return Key::_0;
		case kVK_ANSI_1: return Key::_1;
		case kVK_ANSI_2: return Key::_2;
		case kVK_ANSI_3: return Key::_3;
		case kVK_ANSI_4: return Key::_4;
		case kVK_ANSI_5: return Key::_5;
		case kVK_ANSI_6: return Key::_6;
		case kVK_ANSI_7: return Key::_7;
		case kVK_ANSI_8: return Key::_8;
		case kVK_ANSI_9: return Key::_9;

		case kVK_ANSI_A: return Key::A;
		case kVK_ANSI_B: return Key::B;
		case kVK_ANSI_C: return Key::C;
		case kVK_ANSI_D: return Key::D;
		case kVK_ANSI_E: return Key::E;
		case kVK_ANSI_F: return Key::F;
		case kVK_ANSI_G: return Key::G;
		case kVK_ANSI_H: return Key::H;
		case kVK_ANSI_I: return Key::I;
		case kVK_ANSI_J: return Key::J;
		case kVK_ANSI_K: return Key::K;
		case kVK_ANSI_L: return Key::L;
		case kVK_ANSI_M: return Key::M;
		case kVK_ANSI_N: return Key::N;
		case kVK_ANSI_O: return Key::O;
		case kVK_ANSI_P: return Key::P;
		case kVK_ANSI_Q: return Key::Q;
		case kVK_ANSI_R: return Key::R;
		case kVK_ANSI_S: return Key::S;
		case kVK_ANSI_T: return Key::T;
		case kVK_ANSI_U: return Key::U;
		case kVK_ANSI_V: return Key::V;
		case kVK_ANSI_W: return Key::W;
		case kVK_ANSI_X: return Key::X;
		case kVK_ANSI_Y: return Key::Y;
		case kVK_ANSI_Z: return Key::Z;

		case kVK_F1:  return Key::F1;
		case kVK_F2:  return Key::F2;
		case kVK_F3:  return Key::F3;
		case kVK_F4:  return Key::F4;
		case kVK_F5:  return Key::F5;
		case kVK_F6:  return Key::F6;
		case kVK_F7:  return Key::F7;
		case kVK_F8:  return Key::F8;
		case kVK_F9:  return Key::F9;
		case kVK_F10: return Key::F10;
		case kVK_F11: return Key::F11;
		case kVK_F12: return Key::F12;

		case kVK_Escape:               return Key::Escape;
		case kVK_Tab:                  return Key::Tab;
		case kVK_CapsLock:             return Key::CapsLock;
		case kVK_Shift:                return Key::Shift;
		case kVK_Control:              return Key::Control;
		case optionKey:                return Key::Alt;
		case kVK_Space:                return Key::Space;
		case kVK_Return:               return Key::Return;
		case NSDeleteFunctionKey:      return Key::Backspace;
		case NSPrintScreenFunctionKey: return Key::PrintScreen;
		case NSScrollLockFunctionKey:  return Key::ScrollLock;
		case NSPauseFunctionKey:       return Key::Pause;
		case NSInsertFunctionKey:      return Key::Insert;
		case kVK_Home:                 return Key::Home;
		case kVK_PageUp:               return Key::PageUp;
		case kVK_Delete:               return Key::Delete;
		case kVK_End:                  return Key::End;
		case kVK_PageDown:             return Key::PageDown;
		case NSClearLineFunctionKey:   return Key::NumLock;
		case kVK_ANSI_KeypadDivide:    return Key::Divide;
		case kVK_ANSI_KeypadMultiply:  return Key::Multiply;
		case kVK_ANSI_KeypadMinus:     return Key::Subtract;
		case kVK_ANSI_KeypadPlus:      return Key::Add;
		case kVK_ANSI_KeypadDecimal:   return Key::DecimalPoint;
		case kVK_DownArrow:            return Key::ArrowDown;
		case kVK_LeftArrow:            return Key::ArrowLeft;
		case kVK_RightArrow:           return Key::ArrowRight;
		case kVK_UpArrow:              return Key::ArrowUp;
	}

#elif defined( ORB_OS_ANDROID )

	switch( system_key )
	{
		default: return Key::Unknown;

		case AKEYCODE_0: return Key::_0;
		case AKEYCODE_1: return Key::_1;
		case AKEYCODE_2: return Key::_2;
		case AKEYCODE_3: return Key::_3;
		case AKEYCODE_4: return Key::_4;
		case AKEYCODE_5: return Key::_5;
		case AKEYCODE_6: return Key::_6;
		case AKEYCODE_7: return Key::_7;
		case AKEYCODE_8: return Key::_8;
		case AKEYCODE_9: return Key::_9;

		case AKEYCODE_A: return Key::A;
		case AKEYCODE_B: return Key::B;
		case AKEYCODE_C: return Key::C;
		case AKEYCODE_D: return Key::D;
		case AKEYCODE_E: return Key::E;
		case AKEYCODE_F: return Key::F;
		case AKEYCODE_G: return Key::G;
		case AKEYCODE_H: return Key::H;
		case AKEYCODE_I: return Key::I;
		case AKEYCODE_J: return Key::J;
		case AKEYCODE_K: return Key::K;
		case AKEYCODE_L: return Key::L;
		case AKEYCODE_M: return Key::M;
		case AKEYCODE_N: return Key::N;
		case AKEYCODE_O: return Key::O;
		case AKEYCODE_P: return Key::P;
		case AKEYCODE_Q: return Key::Q;
		case AKEYCODE_R: return Key::R;
		case AKEYCODE_S: return Key::S;
		case AKEYCODE_T: return Key::T;
		case AKEYCODE_U: return Key::U;
		case AKEYCODE_V: return Key::V;
		case AKEYCODE_W: return Key::W;
		case AKEYCODE_X: return Key::X;
		case AKEYCODE_Y: return Key::Y;
		case AKEYCODE_Z: return Key::Z;

		case AKEYCODE_F1:  return Key::F1;
		case AKEYCODE_F2:  return Key::F2;
		case AKEYCODE_F3:  return Key::F3;
		case AKEYCODE_F4:  return Key::F4;
		case AKEYCODE_F5:  return Key::F5;
		case AKEYCODE_F6:  return Key::F6;
		case AKEYCODE_F7:  return Key::F7;
		case AKEYCODE_F8:  return Key::F8;
		case AKEYCODE_F9:  return Key::F9;
		case AKEYCODE_F10: return Key::F10;
		case AKEYCODE_F11: return Key::F11;
		case AKEYCODE_F12: return Key::F12;

		case AKEYCODE_ESCAPE:          return Key::Escape;
		case AKEYCODE_TAB:             return Key::Tab;
		case AKEYCODE_CAPS_LOCK:       return Key::CapsLock;
		case AKEYCODE_SHIFT_LEFT:      return Key::Shift;
		case AKEYCODE_SHIFT_RIGHT:     return Key::Shift;
		case AKEYCODE_CTRL_LEFT:       return Key::Control;
		case AKEYCODE_CTRL_RIGHT:      return Key::Control;
		case AKEYCODE_ALT_RIGHT:       return Key::Alt;
		case AKEYCODE_ALT_LEFT:        return Key::Alt;
		case AKEYCODE_SPACE:           return Key::Space;
		case AKEYCODE_ENTER:           return Key::Return;
		case AKEYCODE_DEL:             return Key::Backspace;
		case AKEYCODE_SYSRQ:           return Key::PrintScreen;
		case AKEYCODE_SCROLL_LOCK:     return Key::ScrollLock;
		case AKEYCODE_BREAK:           return Key::Pause;
		case AKEYCODE_INSERT:          return Key::Insert;
		case AKEYCODE_MOVE_HOME:       return Key::Home;
		case AKEYCODE_PAGE_UP:         return Key::PageUp;
		case AKEYCODE_FORWARD_DEL:     return Key::Delete;
		case AKEYCODE_MOVE_END:        return Key::End;
		case AKEYCODE_PAGE_DOWN:       return Key::PageDown;
		case AKEYCODE_NUM_LOCK:        return Key::NumLock;
		case AKEYCODE_NUMPAD_DIVIDE:   return Key::Divide;
		case AKEYCODE_NUMPAD_MULTIPLY: return Key::Multiply;
		case AKEYCODE_NUMPAD_SUBTRACT: return Key::Subtract;
		case AKEYCODE_NUMPAD_ADD:      return Key::Add;
		case AKEYCODE_NUMPAD_DOT:      return Key::DecimalPoint;
		case AKEYCODE_NUMPAD_COMMA:    return Key::DecimalPoint;
		case AKEYCODE_DPAD_DOWN:       return Key::ArrowDown;
		case AKEYCODE_DPAD_LEFT:       return Key::ArrowLeft;
		case AKEYCODE_DPAD_RIGHT:      return Key::ArrowRight;
		case AKEYCODE_DPAD_UP:         return Key::ArrowUp;

		case AKEYCODE_BACK:   return Key::Back;
		case AKEYCODE_SEARCH: return Key::Search;
	}

#endif

}

ORB_NAMESPACE_END
