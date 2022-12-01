/**
 * @file ShitEnum.hpp
 * @author yangzs
 * @brief
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include <type_traits>
#include <stdexcept>
namespace Shit
{

	/**
	 * https://wiggling-bits.net/using-enum-classes-as-type-safe-bitmasks/
	 */
	template <typename Enum>
	struct EnableBitMaskOperators
	{
		static const bool enable = false;
	};

#define ENABLE_BITMASK_OPERATORS(x)      \
	template <>                          \
	struct EnableBitMaskOperators<x>     \
	{                                    \
		static const bool enable = true; \
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator|(Enum lhs, Enum rhs)
	{
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) | static_cast<std::underlying_type_t<Enum>>(rhs));
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator&(Enum lhs, Enum rhs)
	{
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) & static_cast<std::underlying_type_t<Enum>>(rhs));
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator^(Enum lhs, Enum rhs)
	{
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) ^ static_cast<std::underlying_type_t<Enum>>(rhs));
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator~(Enum rhs)
	{
		return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(rhs));
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	bool operator!(Enum rhs)
	{
		return !static_cast<bool>(rhs);
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum &operator|=(Enum &lhs, Enum rhs)
	{
		lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) | static_cast<std::underlying_type_t<Enum>>(rhs));
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum &operator&=(Enum &lhs, Enum rhs)
	{
		lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) & static_cast<std::underlying_type_t<Enum>>(rhs));
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum &operator^=(Enum &lhs, Enum rhs)
	{
		lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) ^ static_cast<std::underlying_type_t<Enum>>(rhs));
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator>>(Enum lhs, uint32_t offset)
	{
		lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) >> offset);
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum operator<<(Enum lhs, uint32_t offset)
	{
		lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(lhs) << offset);
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum &operator>>=(Enum &lhs, uint32_t offset)
	{
		lhs = lhs >> offset;
		return lhs;
	}

	template <typename Enum, typename std::enable_if_t<std::is_enum_v<Enum> && EnableBitMaskOperators<Enum>::enable, bool> = true>
	Enum &operator<<=(Enum &lhs, uint32_t offset)
	{
		lhs = lhs << offset;
		return lhs;
	}
	enum class WindowCreateFlagBits
	{
		INVISIBLE = 0x1,
		FIXED_SIZE = 0x2,
	};
	ENABLE_BITMASK_OPERATORS(WindowCreateFlagBits);

	enum class EventType
	{
		NONE,
		MOUSEMOVE,
		MOUSEBUTTON,
		MOUSEWHEEL,
		KEYBOARD,
		DROP,
		CHAR,
		WINDOW_CREATE,
		WINDOW_CLOSE,
		WINDOW_DESTROY,
		// WINDOW_QUIT,
		WINDOW_RESIZE,
		WINDOW_CONTENTSACLE,
		WINDOW_POS,
		WINDOW_ICONIFY,
		WINDOW_MAXIMIZE,
		WINDOW_FOCUS,
		WINDOW_REFRESH,
		WINDOW_COUNT
	};
	enum KeyCode
	{
		KEY_LBUTTON, //        0x01
		KEY_RBUTTON, //        0x02
		KEY_CANCEL,	 //         0x03
		KEY_MBUTTON, //        0x04    /* NOT contiguous with L & RBUTTON */

		KEY_BACK, //           0x08
		KEY_TAB,  //            0x09

		KEY_CLEAR,	//          0x0C
		KEY_RETURN, //         0x0D

		KEY_SHIFT,	 //          0x10
		KEY_CONTROL, //        0x11
		KEY_MENU,	 //           0x12
		KEY_PAUSE,	 //          0x13
		KEY_CAPITAL, //        0x14

		KEY_KANA,	 //           0x15
		KEY_HANGEUL, //        0x15  /* old name - should be here for compatibility */
		KEY_HANGUL,	 //         0x15
		// KEY_IME_ON,	 //         0x16
		KEY_JUNJA, //          0x17
		KEY_FINAL, //          0x18
		KEY_HANJA, //          0x19
		KEY_KANJI, //          0x19
		// KEY_IME_OFF, //        0x1A

		KEY_ESCAPE, //         0x1B

		KEY_CONVERT,	//        0x1C
		KEY_NONCONVERT, //     0x1D
		KEY_ACCEPT,		//         0x1E
		KEY_MODECHANGE, //     0x1F

		KEY_SPACE,	  //          0x20
		KEY_PRIOR,	  //          0x21
		KEY_NEXT,	  //           0x22
		KEY_END,	  //            0x23
		KEY_HOME,	  //           0x24
		KEY_LEFT,	  //           0x25
		KEY_UP,		  //             0x26
		KEY_RIGHT,	  //          0x27
		KEY_DOWN,	  //           0x28
		KEY_SELECT,	  //         0x29
		KEY_PRINT,	  //          0x2A
		KEY_EXECUTE,  //        0x2B
		KEY_SNAPSHOT, //       0x2C
		KEY_INSERT,	  //         0x2D
		KEY_DELETE,	  //         0x2E
		KEY_HELP,	  //           0x2F

		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,

		KEY_A,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,

		KEY_LWIN, //           0x5B
		KEY_RWIN, //           0x5C
		KEY_APPS, //           0x5D

		/*
		 * 0x5E : reserved
		 */

		KEY_SLEEP, //          0x5F

		KEY_NUMPAD0,   //        0x60
		KEY_NUMPAD1,   //        0x61
		KEY_NUMPAD2,   //        0x62
		KEY_NUMPAD3,   //        0x63
		KEY_NUMPAD4,   //        0x64
		KEY_NUMPAD5,   //        0x65
		KEY_NUMPAD6,   //        0x66
		KEY_NUMPAD7,   //        0x67
		KEY_NUMPAD8,   //        0x68
		KEY_NUMPAD9,   //        0x69
		KEY_MULTIPLY,  //       0x6A
		KEY_ADD,	   //            0x6B
		KEY_SEPARATOR, //      0x6C
		KEY_SUBTRACT,  //       0x6D
		KEY_DECIMAL,   //        0x6E
		KEY_DIVIDE,	   //         0x6F
		KEY_F1,		   //             0x70
		KEY_F2,		   //             0x71
		KEY_F3,		   //             0x72
		KEY_F4,		   //             0x73
		KEY_F5,		   //             0x74
		KEY_F6,		   //             0x75
		KEY_F7,		   //             0x76
		KEY_F8,		   //             0x77
		KEY_F9,		   //             0x78
		KEY_F10,	   //            0x79
		KEY_F11,	   //            0x7A
		KEY_F12,	   //            0x7B
		KEY_F13,	   //            0x7C
		KEY_F14,	   //            0x7D
		KEY_F15,	   //            0x7E
		KEY_F16,	   //            0x7F
		KEY_F17,	   //            0x80
		KEY_F18,	   //            0x81
		KEY_F19,	   //            0x82
		KEY_F20,	   //            0x83
		KEY_F21,	   //            0x84
		KEY_F22,	   //            0x85
		KEY_F23,	   //            0x86
		KEY_F24,	   //            0x87

		KEY_NUMLOCK, //        0x90
		KEY_SCROLL,	 //         0x91

		/*
		 * NEC PC-9800 kbd definitions
		 */
		KEY_EQUAL, // KEY_OEM_NEC_EQUAL, //  0x92   // '=' key on numpad

		/*
		 * Fujitsu/OASYS kbd definitions
		 */
		KEY_OEM_FJ_JISHO,	//   0x92   // 'Dictionary' key
		KEY_OEM_FJ_MASSHOU, // 0x93   // 'Unregister word' key
		KEY_OEM_FJ_TOUROKU, // 0x94   // 'Register word' key
		KEY_OEM_FJ_LOYA,	//    0x95   // 'Left OYAYUBI' key
		KEY_OEM_FJ_ROYA,	//    0x96   // 'Right OYAYUBI' key

		/*
		 * 0x97 - 0x9F : unassigned
		 */

		/*
		 * KEY_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
		 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
		 * No other API or message will distinguish left and right keys in this way.
		 */
		KEY_LSHIFT,	  //         0xA0
		KEY_RSHIFT,	  //         0xA1
		KEY_LCONTROL, //       0xA2
		KEY_RCONTROL, //       0xA3
		KEY_LMENU,	  //          0xA4
		KEY_RMENU,	  //          0xA5

		/*
		 * 0xB8 - 0xB9 : reserved
		 */

		KEY_SEMICOLON, // KEY_OEM_1,	   //          0xBA   // ';:' for US
		KEY_PLUS,	   // KEY_OEM_PLUS,   //       0xBB   // '+' any country
		KEY_COMMA,	   // KEY_OEM_COMMA,  //      0xBC   // ',' any country
		KEY_MINUS,	   // KEY_OEM_MINUS,  //      0xBD   // '-' any country
		KEY_PERIOD,	   // KEY_OEM_PERIOD, //     0xBE   // '.' any country
		KEY_SLASH,	   // KEY_OEM_2,	   //          0xBF   // '/?' for US
		KEY_TILDE,	   // KEY_OEM_3,	   //          0xC0   // '`~' for US

		/*
		 * 0xC1 - 0xC2 : reserved
		 */

		KEY_LBRACKET,	// KEY_OEM_4, //          0xDB  //  '[{' for US
		KEY_BACK_SLASH, // KEY_OEM_5, //          0xDC  //  '\|' for US
		KEY_RBRACKET,	// KEY_OEM_6, //          0xDD  //  ']}' for US
		KEY_QUOTE,		// KEY_OEM_7, //          0xDE  //  ''"' for US
		KEY_OEM_8,		//          0xDF

		/*
		 * 0xE0 : reserved
		 */

		/*
		 * Various extended or enhanced keyboards
		 */
		KEY_OEM_AX,	  //         0xE1  //  'AX' key on Japanese AX kbd
		KEY_OEM_102,  //        0xE2  //  "<>" or "\|" on RT 102-key kbd.
		KEY_ICO_HELP, //       0xE3  //  Help key on ICO
		KEY_ICO_00,	  //         0xE4  //  00 key on ICO

		KEY_ICO_CLEAR, //      0xE6

		/*
		 * 0xE8 : unassigned
		 */

		/*
		 * Nokia/Ericsson definitions
		 */
		// KEY_OEM_RESET,	//      0xE9
		// KEY_OEM_JUMP,	//       0xEA
		// KEY_OEM_PA1,		//        0xEB
		// KEY_OEM_PA2,		//        0xEC
		// KEY_OEM_PA3,		//        0xED
		// KEY_OEM_WSCTRL,	//     0xEE
		// KEY_OEM_CUSEL,	//      0xEF
		// KEY_OEM_ATTN,	//       0xF0
		// KEY_OEM_FINISH,	//     0xF1
		// KEY_OEM_COPY,	//       0xF2
		// KEY_OEM_AUTO,	//       0xF3
		// KEY_OEM_ENLW,	//       0xF4
		// KEY_OEM_BACKTAB, //    0xF5

		// KEY_ATTN,	  //           0xF6
		// KEY_CRSEL,	  //          0xF7
		// KEY_EXSEL,	  //          0xF8
		// KEY_EREOF,	  //          0xF9
		// KEY_PLAY,	  //           0xFA
		// KEY_ZOOM,	  //           0xFB
		// KEY_NONAME,	  //         0xFC
		// KEY_PA1,		  //            0xFD
		// KEY_OEM_CLEAR, //      0xFE

		KEY_COUNT
	};

	enum class GLSLKeyword
	{
		CONST,
		UNIFORM,
		BUFFER,
		SHARED,
		ATTRIBUTE,
		VARYING,
		COHERENT,
		VOLATILE,
		RESTRICT,
		READONLY,
		WRITEONLY,
		ATOMIC_UINT,
		LAYOUT,
		CENTROID,
		FLAT,
		SMOOTH,
		NOPERSPECTIVE,
		PATCH,
		SAMPLE,
		INVARIANT,
		PRECISE,
		BREAK,
		CONTINUE,
		DO,
		FOR,
		WHILE,
		SWITCH,
		CASE,
		DEFAULT,
		IF,
		ELSE,
		SUBROUTINE,
		IN,
		OUT,
		INOUT,
		INT,
		VOID,
		BOOL,
		TRUE,
		FALSE,
		FLOAT,
		DOUBLE,
		DISCARD,
		RETURN,
		VEC2,
		VEC3,
		VEC4,
		IVEC2,
		IVEC3,
		IVEC4,
		BVEC2,
		BVEC3,
		BVEC4,
		UINT,
		UVEC2,
		UVEC3,
		UVEC4,
		DVEC2,
		DVEC3,
		DVEC4,
		MAT2,
		MAT3,
		MAT4,
		MAT2X2,
		MAT2X3,
		MAT2X4,
		MAT3X2,
		MAT3X3,
		MAT3X4,
		MAT4X2,
		MAT4X3,
		MAT4X4,
		DMAT2,
		DMAT3,
		DMAT4,
		DMAT2X2,
		DMAT2X3,
		DMAT2X4,
		DMAT3X2,
		DMAT3X3,
		DMAT3X4,
		DMAT4X2,
		DMAT4X3,
		DMAT4X4,
		LOWP,
		MEDIUMP,
		HIGHP,
		PRECISION,

		SAMPLER1D,
		SAMPLER1DSHADOW,
		SAMPLER1DARRAY,
		SAMPLER1DARRAYSHADOW,
		ISAMPLER1D,
		ISAMPLER1DARRAY,
		USAMPLER1D,
		USAMPLER1DARRAY,
		SAMPLER2D,
		SAMPLER2DSHADOW,
		SAMPLER2DARRAY,
		SAMPLER2DARRAYSHADOW,
		ISAMPLER2D,
		ISAMPLER2DARRAY,
		USAMPLER2D,
		USAMPLER2DARRAY,
		SAMPLER2DRECT,
		SAMPLER2DRECTSHADOW,
		ISAMPLER2DRECT,
		USAMPLER2DRECT,
		SAMPLER2DMS,
		ISAMPLER2DMS,
		USAMPLER2DMS,
		SAMPLER2DMSARRAY,
		ISAMPLER2DMSARRAY,
		USAMPLER2DMSARRAY,
		SAMPLER3D,
		ISAMPLER3D,
		USAMPLER3D,
		SAMPLERCUBE,
		SAMPLERCUBESHADOW,
		ISAMPLERCUBE,
		USAMPLERCUBE,
		SAMPLERCUBEARRAY,
		SAMPLERCUBEARRAYSHADOW,
		ISAMPLERCUBEARRAY,
		USAMPLERCUBEARRAY,
		SAMPLERBUFFER,
		ISAMPLERBUFFER,
		USAMPLERBUFFER,

		IMAGE1D,
		IIMAGE1D,
		UIMAGE1D,
		IMAGE1DARRAY,
		IIMAGE1DARRAY,
		UIMAGE1DARRAY,
		IMAGE2D,
		IIMAGE2D,
		UIMAGE2D,
		IMAGE2DARRAY,
		IIMAGE2DARRAY,
		UIMAGE2DARRAY,
		IMAGE2DRECT,
		IIMAGE2DRECT,
		UIMAGE2DRECT,
		IMAGE2DMS,
		IIMAGE2DMS,
		UIMAGE2DMS,
		IMAGE2DMSARRAY,
		IIMAGE2DMSARRAY,
		UIMAGE2DMSARRAY,
		IMAGE3D,
		IIMAGE3D,
		UIMAGE3D,
		IMAGECUBE,
		IIMAGECUBE,
		UIMAGECUBE,
		IMAGECUBEARRAY,
		IIMAGECUBEARRAY,
		UIMAGECUBEARRAY,
		IMAGEBUFFER,
		IIMAGEBUFFER,
		UIMAGEBUFFER,
		STRUCT,

		// vulkan
		TEXTURE1D,
		TEXTURE1DARRAY,
		ITEXTURE1D,
		ITEXTURE1DARRAY,
		UTEXTURE1D,
		UTEXTURE1DARRAY,
		TEXTURE2D,
		TEXTURE2DARRAY,
		ITEXTURE2D,
		ITEXTURE2DARRAY,
		UTEXTURE2D,
		UTEXTURE2DARRAY,
		TEXTURE2DRECT,
		ITEXTURE2DRECT,
		UTEXTURE2DRECT,
		TEXTURE2DMS,
		ITEXTURE2DMS,
		UTEXTURE2DMS,
		TEXTURE2DMSARRAY,
		ITEXTURE2DMSARRAY,
		UTEXTURE2DMSARRAY,
		TEXTURE3D,
		ITEXTURE3D,
		UTEXTURE3D,
		TEXTURECUBE,
		ITEXTURECUBE,
		UTEXTURECUBE,
		TEXTURECUBEARRAY,
		ITEXTURECUBEARRAY,
		UTEXTURECUBEARRAY,
		TEXTUREBUFFER,
		ITEXTUREBUFFER,
		UTEXTUREBUFFER,
		SAMPLER,
		SAMPLERSHADOW,
		Num,
	};

	enum class EventModifierBits
	{
		None = 0,
		// ALTL = 0x1,
		CTRLL = 0x2,
		SHIFTL = 0x4,
		META = 0x8,
		BUTTONL = 0x10, // mouse left button
		BUTTONR = 0x20,
		BUTTONM = 0x40,
		BUTTONX1 = 0x80,
		BUTTONX2 = 0x100,
		// ALTR = 0x200,
		CTRLR = 0x400,
		SHIFTR = 0x800,
	};
	ENABLE_BITMASK_OPERATORS(EventModifierBits);

	enum class InputModifierBits
	{
		ALT = 0x1,
		CONTROL = 0x2,
		NOREPEAT = 0x4,
		SHITFT = 0x4,
		WIN = 0x8,
	};
	ENABLE_BITMASK_OPERATORS(InputModifierBits);

	enum class MouseButton
	{
		MOUSE_NONE,
		MOUSE_L,
		MOUSE_M,
		MOUSE_R,
	};

	enum class DataType
	{
		BYTE,
		UNSIGNED_BYTE,
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT,
		FLOAT_HALF,
		DOUBLE,
		// INT_2_10_10_10_REV,
		// UNSIGNED_INT_2_10_10_10_REV,
		// UNSIGNED_INT_10F_11F_11F_REV
		UNSIGNED_INT_24_8,
	};
	static uint32_t dataTypeSizeArray[]{
		1, // BYTE,
		1, // UNSIGNED_BYTE,
		2, // SHORT,
		2, // UNSIGNED_SHORT,
		4, // INT,
		4, // UNSIGNED_INT,
		4, // FLOAT,
		2, // FLOAT_HALF,
		8, // DOUBLE,
		// 1,////INT_2_10_10_10_REV,
		// 1,////UNSIGNED_INT_2_10_10_10_REV,
		// 1,////UNSIGNED_INT_10F_11F_11F_REV
		5, // UNSIGNED_INT_24_8,
	};

	inline uint32_t GetDataTypeSize(DataType type)
	{
		return dataTypeSizeArray[static_cast<size_t>(type)];
	}

	enum class ShadingLanguage
	{
		GLSL = (0x10000),			//!< GLSL (OpenGL Shading Language).
		GLSL_110 = (0x10000 | 110), //!< GLSL 1.10 (since OpenGL 2.0).
		GLSL_120 = (0x10000 | 120), //!< GLSL 1.20 (since OpenGL 2.1).
		GLSL_130 = (0x10000 | 130), //!< GLSL 1.30 (since OpenGL 3.0).
		GLSL_140 = (0x10000 | 140), //!< GLSL 1.40 (since OpenGL 3.1).
		GLSL_150 = (0x10000 | 150), //!< GLSL 1.50 (since OpenGL 3.2).
		GLSL_330 = (0x10000 | 330), //!< GLSL 3.30 (since OpenGL 3.3).
		GLSL_400 = (0x10000 | 400), //!< GLSL 4.00 (since OpenGL 4.0).
		GLSL_410 = (0x10000 | 410), //!< GLSL 4.10 (since OpenGL 4.1).
		GLSL_420 = (0x10000 | 420), //!< GLSL 4.20 (since OpenGL 4.2).
		GLSL_430 = (0x10000 | 430), //!< GLSL 4.30 (since OpenGL 4.3).
		GLSL_440 = (0x10000 | 440), //!< GLSL 4.40 (since OpenGL 4.4).
		GLSL_450 = (0x10000 | 450), //!< GLSL 4.50 (since OpenGL 4.5).
		GLSL_460 = (0x10000 | 460), //!< GLSL 4.60 (since OpenGL 4.6).

		ESSL = (0x20000),			//!< ESSL (OpenGL ES Shading Language).
		ESSL_100 = (0x20000 | 100), //!< ESSL 1.00 (since OpenGL ES 2.0).
		ESSL_300 = (0x20000 | 300), //!< ESSL 3.00 (since OpenGL ES 3.0).
		ESSL_310 = (0x20000 | 310), //!< ESSL 3.10 (since OpenGL ES 3.1).
		ESSL_320 = (0x20000 | 320), //!< ESSL 3.20 (since OpenGL ES 3.2).

		HLSL = (0x30000),			 //!< HLSL (High Level Shading Language).
		HLSL_2_0 = (0x30000 | 200),	 //!< HLSL 2.0 (since Direct3D 9).
		HLSL_2_0a = (0x30000 | 201), //!< HLSL 2.0a (since Direct3D 9a).
		HLSL_2_0b = (0x30000 | 202), //!< HLSL 2.0b (since Direct3D 9b).
		HLSL_3_0 = (0x30000 | 300),	 //!< HLSL 3.0 (since Direct3D 9c).
		HLSL_4_0 = (0x30000 | 400),	 //!< HLSL 4.0 (since Direct3D 10).
		HLSL_4_1 = (0x30000 | 410),	 //!< HLSL 4.1 (since Direct3D 10.1).
		HLSL_5_0 = (0x30000 | 500),	 //!< HLSL 5.0 (since Direct3D 11).
		HLSL_5_1 = (0x30000 | 510),	 //!< HLSL 5.1 (since Direct3D 11.3).
		HLSL_6_0 = (0x30000 | 600),	 //!< HLSL 6.0 (since Direct3D 12). Shader model 6.0 adds wave intrinsics and 64-bit integer types to HLSL.
		HLSL_6_1 = (0x30000 | 601),	 //!< HLSL 6.1 (since Direct3D 12). Shader model 6.1 adds \c SV_ViewID and \c SV_Barycentrics semantics to HLSL.
		HLSL_6_2 = (0x30000 | 602),	 //!< HLSL 6.2 (since Direct3D 12). Shader model 6.2 adds 16-bit scalar types to HLSL.
		HLSL_6_3 = (0x30000 | 603),	 //!< HLSL 6.3 (since Direct3D 12). Shader model 6.3 adds ray tracing (DXR) to HLSL.
		HLSL_6_4 = (0x30000 | 604),	 //!< HLSL 6.4 (since Direct3D 12). Shader model 6.4 adds machine learning intrinsics to HLSL.

		Metal = (0x40000),			 //!< Metal Shading Language.
		Metal_1_0 = (0x40000 | 100), //!< Metal 1.0 (since iOS 8.0).
		Metal_1_1 = (0x40000 | 110), //!< Metal 1.1 (since iOS 9.0 and OS X 10.11).
		Metal_1_2 = (0x40000 | 120), //!< Metal 1.2 (since iOS 10.0 and macOS 10.12).
		Metal_2_0 = (0x40000 | 200), //!< Metal 2.0 (since iOS 11.0 and macOS 10.13).
		Metal_2_1 = (0x40000 | 210), //!< Metal 2.1 (since iOS 12.0 and macOS 10.14).

		SPIRV = (0x50000),			 //!< SPIR-V Shading Language.
		SPIRV_100 = (0x50000 | 100), //!< SPIR-V 1.0.
		SPIRV_110 = (0x50000 | 110), //!< SPIR-V 1.1.
		SPIRV_120 = (0x50000 | 120), //!< SPIR-V 1.2.
		SPIRV_130 = (0x50000 | 130), //!< SPIR-V 1.3.
		SPIRV_140 = (0x50000 | 140), //!< SPIR-V 1.4. May 7, 2019
		SPIRV_150 = (0x50000 | 150), //!< SPIR-V 1.5. Sep 13, 2019

		VersionBitmask = 0x0000ffff, //!< Bitmask for the version number of each shading language enumeration entry.
		TypeBitmask = 0xf0000,		 //!< Bitmask for shader type
	};
	ENABLE_BITMASK_OPERATORS(ShadingLanguage);

	enum class ShaderBinaryFormat
	{
		SPIR_V,
	};

	// same as vulkan
	enum class ShaderStageFlagBits
	{
		VERTEX_BIT = 0x00000001,
		TESSELLATION_CONTROL_BIT = 0x00000002,
		TESSELLATION_EVALUATION_BIT = 0x00000004,
		GEOMETRY_BIT = 0x00000008,
		FRAGMENT_BIT = 0x00000010,
		COMPUTE_BIT = 0x00000020,
		ALL_GRAPHICS = 0x0000001F,
		ALL = 0x7FFFFFFF,
		RAYGEN_BIT = 0x00000100,
		ANY_HIT_BIT = 0x00000200,
		CLOSEST_HIT_BIT = 0x00000400,
		MISS_BIT = 0x00000800,
		INTERSECTION_BIT = 0x00001000,
		CALLABLE_BIT = 0x00002000,
		TASK_BIT = 0x00000040,
		MESH_BIT = 0x00000080,
	};
	ENABLE_BITMASK_OPERATORS(ShaderStageFlagBits);

	enum class RendererVersion
	{
		GL = (0x10000),				//!< OpenGL lastest.
		GL_110 = (0x10000 | 0x110), //!< OpenGL 1.1.	1997
		GL_120 = (0x10000 | 0x120), //!< OpenGL 1.2.	1998
		GL_121 = (0x10000 | 0x121), //!< OpenGL 1.2.1	1998
		GL_130 = (0x10000 | 0x130), //!< OpenGL 1.3.	2001
		GL_140 = (0x10000 | 0x140), //!< OpenGL 1.4.	2002
		GL_150 = (0x10000 | 0x150), //!< OpenGL 1.5.	2003
		GL_200 = (0x10000 | 0x200), //!< OpenGL 2.0.	2004
		GL_210 = (0x10000 | 0x210), //!< OpenGL 2.1.	2006
		GL_300 = (0x10000 | 0x300), //!< OpenGL 3.0.	2008
		GL_310 = (0x10000 | 0x310), //!< OpenGL 3.1.	2009
		GL_320 = (0x10000 | 0x320), //!< OpenGL 3.2.	2009
		GL_330 = (0x10000 | 0x330), //!< OpenGL 3.3.	2010
		GL_400 = (0x10000 | 0x400), //!< OpenGL 4.0.	2010
		GL_410 = (0x10000 | 0x410), //!< OpenGL 4.1.	2010
		GL_420 = (0x10000 | 0x420), //!< OpenGL 4.2.	2011
		GL_430 = (0x10000 | 0x430), //!< OpenGL 4.3.	2012
		GL_440 = (0x10000 | 0x440), //!< OpenGL 4.4.	2013
		GL_450 = (0x10000 | 0x450), //!< OpenGL 4.5.	2014
		GL_460 = (0x10000 | 0x460), //!< OpenGL 4.6.	2017

		GLES = (0x20000),			  //!< OpenGL ES lastest
		GLES_100 = (0x20000 | 0x100), //!< OpenGL ES 1.0.
		GLES_120 = (0x20000 | 0x200), //!< OpenGL ES 2.0.
		GLES_300 = (0x20000 | 0x300), //!< OpenGL ES 3.0.
		GLES_310 = (0x20000 | 0x310), //!< OpenGL ES 3.1.
		GLES_320 = (0x20000 | 0x320), //!< OpenGL ES 3.2.

		VULKAN = (0x30000),				//!< Vulkan lastest
		VULKAN_100 = (0x30000 | 0x100), //!< Vulkan 1.0
		VULKAN_110 = (0x30000 | 0x110), //!< Vulkan 1.1
		VULKAN_120 = (0x30000 | 0x120), //!< Vulkan 1.2
		VULKAN_130 = (0x30000 | 0x130), //!< Vulkan 1.3
		VULKAN_140 = (0x30000 | 0x140), //!< Vulkan 1.4
		VULKAN_150 = (0x30000 | 0x150), //!< Vulkan 1.5

		METAL = (0x40000),
		D3D11 = (0x50000),
		D3D12 = (0x60000),

		VersionBitmask = 0x0000ffff, //!< Bitmask for the version number of each shading language enumeration entry.
		TypeBitmask = 0xf0000,		 //!< Typemask for renderer check
	};
	ENABLE_BITMASK_OPERATORS(RendererVersion);

	enum class RenderSystemCreateFlagBits
	{
		None = 0,
		CONTEXT_DEBUG_BIT = (0x1),
		GLCONTEXT_FORWARD_COMPATIBLE_BIT = (0x2),
		GLCONTEXT_CORE_PROFILE_BIT = (0x4), //!< default for opengl
		GLCONTEXT_COMPATIBILITY_PROFILE_BIT = (0x8),
	};
	ENABLE_BITMASK_OPERATORS(RenderSystemCreateFlagBits);

	enum class ColorSpace
	{
		SRGB_NONLINEAR,
		Num
	};

	// from vulkan
	enum class Format
	{
		UNDEFINED,									// = 0,
		R4G4_UNORM_PACK8,							// = 1,
		R4G4B4A4_UNORM_PACK16,						// = 2,
		B4G4R4A4_UNORM_PACK16,						// = 3,
		R5G6B5_UNORM_PACK16,						// = 4,
		B5G6R5_UNORM_PACK16,						// = 5,
		R5G5B5A1_UNORM_PACK16,						// = 6,
		B5G5R5A1_UNORM_PACK16,						// = 7,
		A1R5G5B5_UNORM_PACK16,						// = 8,
		R8_UNORM,									// = 9,
		R8_SNORM,									// = 10,
		R8_USCALED,									// = 11,
		R8_SSCALED,									// = 12,
		R8_UINT,									// = 13,
		R8_SINT,									// = 14,
		R8_SRGB,									// = 15,
		R8G8_UNORM,									// = 16,
		R8G8_SNORM,									// = 17,
		R8G8_USCALED,								// = 18,
		R8G8_SSCALED,								// = 19,
		R8G8_UINT,									// = 20,
		R8G8_SINT,									// = 21,
		R8G8_SRGB,									// = 22,
		R8G8B8_UNORM,								// = 23,
		R8G8B8_SNORM,								// = 24,
		R8G8B8_USCALED,								// = 25,
		R8G8B8_SSCALED,								// = 26,
		R8G8B8_UINT,								// = 27,
		R8G8B8_SINT,								// = 28,
		R8G8B8_SRGB,								// = 29,
		B8G8R8_UNORM,								// = 30,
		B8G8R8_SNORM,								// = 31,
		B8G8R8_USCALED,								// = 32,
		B8G8R8_SSCALED,								// = 33,
		B8G8R8_UINT,								// = 34,
		B8G8R8_SINT,								// = 35,
		B8G8R8_SRGB,								// = 36,
		R8G8B8A8_UNORM,								// = 37,
		R8G8B8A8_SNORM,								// = 38,
		R8G8B8A8_USCALED,							// = 39,
		R8G8B8A8_SSCALED,							// = 40,
		R8G8B8A8_UINT,								// = 41,
		R8G8B8A8_SINT,								// = 42,
		R8G8B8A8_SRGB,								// = 43,
		B8G8R8A8_UNORM,								// = 44,
		B8G8R8A8_SNORM,								// = 45,
		B8G8R8A8_USCALED,							// = 46,
		B8G8R8A8_SSCALED,							// = 47,
		B8G8R8A8_UINT,								// = 48,
		B8G8R8A8_SINT,								// = 49,
		B8G8R8A8_SRGB,								// = 50,
		A8B8G8R8_UNORM_PACK32,						// = 51,
		A8B8G8R8_SNORM_PACK32,						// = 52,
		A8B8G8R8_USCALED_PACK32,					// = 53,
		A8B8G8R8_SSCALED_PACK32,					// = 54,
		A8B8G8R8_UINT_PACK32,						// = 55,
		A8B8G8R8_SINT_PACK32,						// = 56,
		A8B8G8R8_SRGB_PACK32,						// = 57,
		A2R10G10B10_UNORM_PACK32,					// = 58,
		A2R10G10B10_SNORM_PACK32,					// = 59,
		A2R10G10B10_USCALED_PACK32,					// = 60,
		A2R10G10B10_SSCALED_PACK32,					// = 61,
		A2R10G10B10_UINT_PACK32,					// = 62,
		A2R10G10B10_SINT_PACK32,					// = 63,
		A2B10G10R10_UNORM_PACK32,					// = 64,
		A2B10G10R10_SNORM_PACK32,					// = 65,
		A2B10G10R10_USCALED_PACK32,					// = 66,
		A2B10G10R10_SSCALED_PACK32,					// = 67,
		A2B10G10R10_UINT_PACK32,					// = 68,
		A2B10G10R10_SINT_PACK32,					// = 69,
		R16_UNORM,									// = 70,
		R16_SNORM,									// = 71,
		R16_USCALED,								// = 72,
		R16_SSCALED,								// = 73,
		R16_UINT,									// = 74,
		R16_SINT,									// = 75,
		R16_SFLOAT,									// = 76,
		R16G16_UNORM,								// = 77,
		R16G16_SNORM,								// = 78,
		R16G16_USCALED,								// = 79,
		R16G16_SSCALED,								// = 80,
		R16G16_UINT,								// = 81,
		R16G16_SINT,								// = 82,
		R16G16_SFLOAT,								// = 83,
		R16G16B16_UNORM,							// = 84,
		R16G16B16_SNORM,							// = 85,
		R16G16B16_USCALED,							// = 86,
		R16G16B16_SSCALED,							// = 87,
		R16G16B16_UINT,								// = 88,
		R16G16B16_SINT,								// = 89,
		R16G16B16_SFLOAT,							// = 90,
		R16G16B16A16_UNORM,							// = 91,
		R16G16B16A16_SNORM,							// = 92,
		R16G16B16A16_USCALED,						// = 93,
		R16G16B16A16_SSCALED,						// = 94,
		R16G16B16A16_UINT,							// = 95,
		R16G16B16A16_SINT,							// = 96,
		R16G16B16A16_SFLOAT,						// = 97,
		R32_UINT,									// = 98,
		R32_SINT,									// = 99,
		R32_SFLOAT,									// = 100,
		R32G32_UINT,								// = 101,
		R32G32_SINT,								// = 102,
		R32G32_SFLOAT,								// = 103,
		R32G32B32_UINT,								// = 104,
		R32G32B32_SINT,								// = 105,
		R32G32B32_SFLOAT,							// = 106,
		R32G32B32A32_UINT,							// = 107,
		R32G32B32A32_SINT,							// = 108,
		R32G32B32A32_SFLOAT,						// = 109,
		R64_UINT,									// = 110,
		R64_SINT,									// = 111,
		R64_SFLOAT,									// = 112,
		R64G64_UINT,								// = 113,
		R64G64_SINT,								// = 114,
		R64G64_SFLOAT,								// = 115,
		R64G64B64_UINT,								// = 116,
		R64G64B64_SINT,								// = 117,
		R64G64B64_SFLOAT,							// = 118,
		R64G64B64A64_UINT,							// = 119,
		R64G64B64A64_SINT,							// = 120,
		R64G64B64A64_SFLOAT,						// = 121,
		B10G11R11_UFLOAT_PACK32,					// = 122,
		E5B9G9R9_UFLOAT_PACK32,						// = 123,
		D16_UNORM,									// = 124,
		X8_D24_UNORM_PACK32,						// = 125,
		D32_SFLOAT,									// = 126,
		S8_UINT,									// = 127,
		D16_UNORM_S8_UINT,							// = 128,
		D24_UNORM_S8_UINT,							// = 129,
		D32_SFLOAT_S8_UINT,							// = 130,
		BC1_RGB_UNORM_BLOCK,						// = 131,
		BC1_RGB_SRGB_BLOCK,							// = 132,
		BC1_RGBA_UNORM_BLOCK,						// = 133,
		BC1_RGBA_SRGB_BLOCK,						// = 134,
		BC2_UNORM_BLOCK,							// = 135,
		BC2_SRGB_BLOCK,								// = 136,
		BC3_UNORM_BLOCK,							// = 137,
		BC3_SRGB_BLOCK,								// = 138,
		BC4_UNORM_BLOCK,							// = 139,
		BC4_SNORM_BLOCK,							// = 140,
		BC5_UNORM_BLOCK,							// = 141,
		BC5_SNORM_BLOCK,							// = 142,
		BC6H_UFLOAT_BLOCK,							// = 143,
		BC6H_SFLOAT_BLOCK,							// = 144,
		BC7_UNORM_BLOCK,							// = 145,
		BC7_SRGB_BLOCK,								// = 146,
		ETC2_R8G8B8_UNORM_BLOCK,					// = 147,
		ETC2_R8G8B8_SRGB_BLOCK,						// = 148,
		ETC2_R8G8B8A1_UNORM_BLOCK,					// = 149,
		ETC2_R8G8B8A1_SRGB_BLOCK,					// = 150,
		ETC2_R8G8B8A8_UNORM_BLOCK,					// = 151,
		ETC2_R8G8B8A8_SRGB_BLOCK,					// = 152,
		EAC_R11_UNORM_BLOCK,						// = 153,
		EAC_R11_SNORM_BLOCK,						// = 154,
		EAC_R11G11_UNORM_BLOCK,						// = 155,
		EAC_R11G11_SNORM_BLOCK,						// = 156,
		ASTC_4x4_UNORM_BLOCK,						// = 157,
		ASTC_4x4_SRGB_BLOCK,						// = 158,
		ASTC_5x4_UNORM_BLOCK,						// = 159,
		ASTC_5x4_SRGB_BLOCK,						// = 160,
		ASTC_5x5_UNORM_BLOCK,						// = 161,
		ASTC_5x5_SRGB_BLOCK,						// = 162,
		ASTC_6x5_UNORM_BLOCK,						// = 163,
		ASTC_6x5_SRGB_BLOCK,						// = 164,
		ASTC_6x6_UNORM_BLOCK,						// = 165,
		ASTC_6x6_SRGB_BLOCK,						// = 166,
		ASTC_8x5_UNORM_BLOCK,						// = 167,
		ASTC_8x5_SRGB_BLOCK,						// = 168,
		ASTC_8x6_UNORM_BLOCK,						// = 169,
		ASTC_8x6_SRGB_BLOCK,						// = 170,
		ASTC_8x8_UNORM_BLOCK,						// = 171,
		ASTC_8x8_SRGB_BLOCK,						// = 172,
		ASTC_10x5_UNORM_BLOCK,						// = 173,
		ASTC_10x5_SRGB_BLOCK,						// = 174,
		ASTC_10x6_UNORM_BLOCK,						// = 175,
		ASTC_10x6_SRGB_BLOCK,						// = 176,
		ASTC_10x8_UNORM_BLOCK,						// = 177,
		ASTC_10x8_SRGB_BLOCK,						// = 178,
		ASTC_10x10_UNORM_BLOCK,						// = 179,
		ASTC_10x10_SRGB_BLOCK,						// = 180,
		ASTC_12x10_UNORM_BLOCK,						// = 181,
		ASTC_12x10_SRGB_BLOCK,						// = 182,
		ASTC_12x12_UNORM_BLOCK,						// = 183,
		ASTC_12x12_SRGB_BLOCK,						// = 184,
		G8B8G8R8_422_UNORM,							// = 1000156000,
		B8G8R8G8_422_UNORM,							// = 1000156001,
		G8_B8_R8_3PLANE_420_UNORM,					// = 1000156002,
		G8_B8R8_2PLANE_420_UNORM,					// = 1000156003,
		G8_B8_R8_3PLANE_422_UNORM,					// = 1000156004,
		G8_B8R8_2PLANE_422_UNORM,					// = 1000156005,
		G8_B8_R8_3PLANE_444_UNORM,					// = 1000156006,
		R10X6_UNORM_PACK16,							// = 1000156007,
		R10X6G10X6_UNORM_2PACK16,					// = 1000156008,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16,			// = 1000156009,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,		// = 1000156010,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,		// = 1000156011,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, // = 1000156012,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,	// = 1000156013,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, // = 1000156014,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,	// = 1000156015,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, // = 1000156016,
		R12X4_UNORM_PACK16,							// = 1000156017,
		R12X4G12X4_UNORM_2PACK16,					// = 1000156018,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16,			// = 1000156019,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,		// = 1000156020,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,		// = 1000156021,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, // = 1000156022,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,	// = 1000156023,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, // = 1000156024,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,	// = 1000156025,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, // = 1000156026,
		G16B16G16R16_422_UNORM,						// = 1000156027,
		B16G16R16G16_422_UNORM,						// = 1000156028,
		G16_B16_R16_3PLANE_420_UNORM,				// = 1000156029,
		G16_B16R16_2PLANE_420_UNORM,				// = 1000156030,
		G16_B16_R16_3PLANE_422_UNORM,				// = 1000156031,
		G16_B16R16_2PLANE_422_UNORM,				// = 1000156032,
		G16_B16_R16_3PLANE_444_UNORM,				// = 1000156033,
		PVRTC1_2BPP_UNORM_BLOCK_IMG,				// = 1000054000,
		PVRTC1_4BPP_UNORM_BLOCK_IMG,				// = 1000054001,
		PVRTC2_2BPP_UNORM_BLOCK_IMG,				// = 1000054002,
		PVRTC2_4BPP_UNORM_BLOCK_IMG,				// = 1000054003,
		PVRTC1_2BPP_SRGB_BLOCK_IMG,					// = 1000054004,
		PVRTC1_4BPP_SRGB_BLOCK_IMG,					// = 1000054005,
		PVRTC2_2BPP_SRGB_BLOCK_IMG,					// = 1000054006,
		PVRTC2_4BPP_SRGB_BLOCK_IMG,					// = 1000054007,
		ASTC_4x4_SFLOAT_BLOCK_EXT,					// = 1000066000,
		ASTC_5x4_SFLOAT_BLOCK_EXT,					// = 1000066001,
		ASTC_5x5_SFLOAT_BLOCK_EXT,					// = 1000066002,
		ASTC_6x5_SFLOAT_BLOCK_EXT,					// = 1000066003,
		ASTC_6x6_SFLOAT_BLOCK_EXT,					// = 1000066004,
		ASTC_8x5_SFLOAT_BLOCK_EXT,					// = 1000066005,
		ASTC_8x6_SFLOAT_BLOCK_EXT,					// = 1000066006,
		ASTC_8x8_SFLOAT_BLOCK_EXT,					// = 1000066007,
		ASTC_10x5_SFLOAT_BLOCK_EXT,					// = 1000066008,
		ASTC_10x6_SFLOAT_BLOCK_EXT,					// = 1000066009,
		ASTC_10x8_SFLOAT_BLOCK_EXT,					// = 1000066010,
		ASTC_10x10_SFLOAT_BLOCK_EXT,				// = 1000066011,
		ASTC_12x10_SFLOAT_BLOCK_EXT,				// = 1000066012,
		ASTC_12x12_SFLOAT_BLOCK_EXT,				// = 1000066013,
		A4R4G4B4_UNORM_PACK16_EXT,					// = 1000340000,
		A4B4G4R4_UNORM_PACK16_EXT,					// = 1000340001,
		Num
	};
	enum class FormatCompressionScheme
	{
		NONE,
		BC,
		ETC2,
		EAC,
		ASTC,
	};
	enum class FormatNumeric
	{
		UNORM,
		SNORM,
		USCALED,
		SSCALED,
		UINT,
		SINT,
		UFLOAT,
		SFLOAT,
		SRGB,
	};
	enum class BaseFormat
	{
		R,
		RG,
		RGB,
		RGBA,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL,
		BGR,
		BGRA,
	};
	struct FormatAttribute
	{
		const char *name;
		FormatNumeric formatNumeric;
		BaseFormat baseFormat;
		uint32_t componentSizeR;
		uint32_t componentSizeG;
		uint32_t componentSizeB;
		uint32_t componentSizeA;
		uint32_t componentSizeDepth;
		uint32_t componentSizeStencil;
		uint32_t componentSizeX;
		FormatCompressionScheme compressionScheme;
	};

	enum class PresentMode
	{
		IMMEDIATE,	  // no vsync, may cause tearing
		MAILBOX,	  // vsync, when queue is full, reuse all entry, (BEST,similar to triple buffering)
		FIFO,		  // vsync, when queue if full wait, (device must support this mode)
		FIFO_RELAXED, // vsync, when queue if full wait, when queue is empty, do not wait VBI, swap immediately, may cause tearing
		SHARED_DEMAND_REFRESH,
		SHARED_CONTINUOUS_REFRESH,
		Num
	};

	enum class DescriptorType
	{
		SAMPLER,				// sampler (vulkan)
		COMBINED_IMAGE_SAMPLER, // sampler2D
		SAMPLED_IMAGE,			// texture2D (vulkan)
		STORAGE_IMAGE,			// image2D
		UNIFORM_TEXEL_BUFFER,	// samplerbuffer	(access to buffer texture,can only be accessed with texelFetch function) ,textureBuffer(vulkan)
		STORAGE_TEXEL_BUFFER,	// imagebuffer (access to buffer texture)
		UNIFORM_BUFFER,			// uniform block
		STORAGE_BUFFER,			// buffer block
		UNIFORM_BUFFER_DYNAMIC,
		STORAGE_BUFFER_DYNAMIC,
		INPUT_ATTACHMENT,
		Num,
		None = 0xFFFF,
	};

	enum class BufferUsageFlagBits
	{
		TRANSFER_SRC_BIT = 0x1,
		TRANSFER_DST_BIT = 0x2,
		UNIFORM_TEXEL_BUFFER_BIT = 0x4,
		STORAGE_TEXEL_BUFFER_BIT = 0x8,
		UNIFORM_BUFFER_BIT = 0x10,
		STORAGE_BUFFER_BIT = 0x20,
		INDEX_BUFFER_BIT = 0x40,
		VERTEX_BUFFER_BIT = 0x80,
		INDIRECT_BUFFER_BIT = 0x100,
		TRANSFORM_FEEDBACK_BUFFER_BIT = 0x200,
	};
	ENABLE_BITMASK_OPERATORS(BufferUsageFlagBits);

	enum class BufferStorageFlagBits
	{
		MAP_READ_BIT = 0x1,
		MAP_WRITE_BIT = 0x2,
		MAP_PERSISTENT_BIT = 0x40,
		MAP_COHERENT_BIT = 0x80,
		DYNAMIC_STORAGE_BIT = 0x100,
		CLIENT_STORAGE_BIT = 0x200,
	};
	ENABLE_BITMASK_OPERATORS(BufferStorageFlagBits);

	enum class BufferMutableStorageUsage
	{
		STREAM_DRAW,
		STREAM_READ,
		STREAM_COPY,
		DYNAMIC_DRAW,
		DYNAMIC_READ,
		DYNAMIC_COPY,
		STATIC_DRAW,
		STATIC_READ,
		STATIC_COPY,
	};
	enum class BufferMapFlagBits
	{
		READ_BIT = 0x1,
		WRITE_BIT = 0x2,
		INVALIDATE_RANGE_BIT = 0x4,
		INVALIDATE_BUFFER_BIT = 0x8,
		FLUSH_EXPLICIT_BIT = 0x10,
		UNSYNCHRONIZED_BIT = 0x20,
		PERSISTENT_BIT = 0x40,
		COHERENT_BIT = 0x80,
	};
	ENABLE_BITMASK_OPERATORS(BufferMapFlagBits);

	enum class MemoryPropertyFlagBits
	{
		DEVICE_LOCAL_BIT = 0x1,
		HOST_VISIBLE_BIT = 0x2,		  // memory canbe mapped
		HOST_COHERENT_BIT = 0x4,	  // flush and invalidate are unnecessary
		HOST_CACHED_BIT = 0x00000008, // faster to access than uncached memory, uncached memory is always coheret
									  // LAZILY_ALLOCATED_BIT = 0x00000010,//only allow device access, cannot use with HOST_VISIBLE
									  // PROTECTED_BIT = 0x00000020,
									  // DEVICE_COHERENT_BIT_AMD = 0x00000040,
									  // DEVICE_UNCACHED_BIT_AMD = 0x00000080,
	};
	ENABLE_BITMASK_OPERATORS(MemoryPropertyFlagBits);

	enum class MemoryMapFlagBits
	{
		READ_BIT = 0x1,
		WRITE_BIT = 0x2,
	};

	enum class BufferCreateFlagBits
	{
		MUTABLE_FORMAT_BIT = 0x1, // for opengl
	};
	ENABLE_BITMASK_OPERATORS(BufferCreateFlagBits);

	enum class ImageCreateFlagBits
	{
		SPARSE_BINDING_BIT = 0x00000001,
		SPARSE_RESIDENCY_BIT = 0x00000002,
		SPARSE_ALIASED_BIT = 0x00000004,
		MUTABLE_FORMAT_BIT = 0x00000008, // mean the format of imageview canbe different from the original, different from GL_IMMUTABLE_FORMAT_BIT
		CUBE_COMPATIBLE_BIT = 0x00000010,
		ALIAS_BIT = 0x00000400,
		SPLIT_INSTANCE_BIND_REGIONS_BIT = 0x00000040,
		ARRAY_2D_COMPATIBLE_BIT = 0x00000020,
		BLOCK_TEXEL_VIEW_COMPATIBLE_BIT = 0x00000080,
		EXTENDED_USAGE_BIT = 0x00000100,
		PROTECTED_BIT = 0x00000800,
		DISJOINT_BIT = 0x00000200,
		CORNER_SAMPLED_BIT_NV = 0x00002000,
		SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT = 0x00001000,
		SUBSAMPLED_BIT_EXT = 0x00004000,
	};
	ENABLE_BITMASK_OPERATORS(ImageCreateFlagBits);

	enum class ImageType
	{
		TYPE_1D,
		TYPE_2D,
		TYPE_3D,
	};

	enum class ImageViewType
	{
		TYPE_1D,
		TYPE_2D,
		TYPE_3D,
		TYPE_CUBE,
		TYPE_1D_ARRAY,
		TYPE_2D_ARRAY,
		TYPE_CUBE_ARRAY,
	};
	enum class ImageTiling
	{
		OPTIMAL,
		LINEAR,
	};

	// same as vulkan
	enum class ImageLayout
	{
		UNDEFINED,
		GENERAL,
		COLOR_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		SHADER_READ_ONLY_OPTIMAL,
		TRANSFER_SRC_OPTIMAL,
		TRANSFER_DST_OPTIMAL,
		PREINITIALIZED,
		DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
		DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
		DEPTH_ATTACHMENT_OPTIMAL,
		DEPTH_READ_ONLY_OPTIMAL,
		STENCIL_ATTACHMENT_OPTIMAL,
		STENCIL_READ_ONLY_OPTIMAL,
		PRESENT_SRC,
		SHARED_PRESENT,
		SHADING_RATE_OPTIMAL,
		FRAGMENT_DENSITY_MAP_OPTIMAL,
	};

	enum class ImageUsageFlagBits
	{
		TRANSFER_SRC_BIT = 0x1,
		TRANSFER_DST_BIT = 0x2,
		SAMPLED_BIT = 0x4,
		STORAGE_BIT = 0x8,
		COLOR_ATTACHMENT_BIT = 0x10,
		DEPTH_STENCIL_ATTACHMENT_BIT = 0x20,
		TRANSIENT_ATTACHMENT_BIT = 0x40, // rendererbuffer for opengl
		INPUT_ATTACHMENT_BIT = 0x80
	};
	ENABLE_BITMASK_OPERATORS(ImageUsageFlagBits);

	// same as vulkan
	enum class ImageAspectFlagBits
	{
		COLOR_BIT = 0x00000001,
		DEPTH_BIT = 0x00000002,
		STENCIL_BIT = 0x00000004,
		METADATA_BIT = 0x00000008,
		PLANE_0_BIT = 0x00000010,
		PLANE_1_BIT = 0x00000020,
		PLANE_2_BIT = 0x00000040,
		MEMORY_PLANE_0_BIT_EXT = 0x00000080,
		MEMORY_PLANE_1_BIT_EXT = 0x00000100,
		MEMORY_PLANE_2_BIT_EXT = 0x00000200,
		MEMORY_PLANE_3_BIT_EXT = 0x00000400,
	};
	ENABLE_BITMASK_OPERATORS(ImageAspectFlagBits);

	enum class SampleCountFlagBits
	{
		BIT_1 = 0x00000001,
		BIT_2 = 0x00000002,
		BIT_4 = 0x00000004,
		BIT_8 = 0x00000008,
		BIT_16 = 0x00000010,
		BIT_32 = 0x00000020,
		BIT_64 = 0x00000040,
	};

	enum class Filter
	{
		NEAREST,
		LINEAR,
	};

	enum SamplerMipmapMode
	{
		NEAREST,
		LINEAR,
	};

	enum class SamplerWrapMode
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
	};
	enum class CompareOp
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_OR_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_OR_EQUAL,
		ALWAYS,
	};

	enum class ComponentSwizzle
	{
		IDENTITY,
		ZERO,
		ONE,
		R,
		G,
		B,
		A,
	};

	enum class AttachmentLoadOp
	{
		LOAD,
		CLEAR, // glClear
		DONT_CARE,
	};

	enum class AttachmentStoreOp
	{
		STORE, // where to store
		DONT_CARE,
	};

	enum class PipelineBindPoint
	{
		GRAPHICS,
		COMPUTE,
	};

	enum class CommandPoolCreateFlagBits
	{
		TRANSIENT_BIT = 0x1,
		RESET_COMMAND_BUFFER_BIT = 0x2,
	};
	ENABLE_BITMASK_OPERATORS(CommandPoolCreateFlagBits);

	enum class CommandBufferLevel
	{
		PRIMARY,
		SECONDARY
	};
	enum class CommandBufferUsageFlagBits
	{
		ONE_TIME_SUBMIT_BIT = 0x1,
		RENDER_PASS_CONTINUE_BIT = 0x2,
		SIMULTANEOUS_USE_BIT = 0x4,
	};
	ENABLE_BITMASK_OPERATORS(CommandBufferUsageFlagBits);

	enum class QueueFlagBits
	{
		GRAPHICS_BIT = 0x1,
		COMPUTE_BIT = 0x2,
		TRANSFER_BIT = 0x4,
		SPARSE_BINDING_BIT = 0x8,
		PROTECTED_BIT = 0x10,
	};
	ENABLE_BITMASK_OPERATORS(QueueFlagBits);

	enum class Result
	{
		SUCCESS,
		NOT_READY,
		TIMEOUT,
		EVENT_SET,
		EVENT_RESET,
		INCOMPLETE,
		SHIT_ERROR,
		SHIT_ERROR_OUT_OF_DATE,
		SUBOPTIMAL,
	};
	enum class SubpassContents
	{
		INLINE,
		SECONDARY_COMMAND_BUFFERS
	};
	enum class IndexType
	{
		NONE,
		UINT8,
		UINT16,
		UINT32,
	};

	inline uint32_t GetIndexTypeSize(IndexType type)
	{
		switch (type)
		{
		case IndexType::UINT8:
			return 1;
		case IndexType::UINT16:
			return 2;
		case IndexType::UINT32:
			return 4;
		default:
			throw std::runtime_error("unknown index type");
		}
	}

	enum class CommandBufferResetFlatBits
	{
		RELEASE_RESOURCES_BIT = 0x1,
	};
	ENABLE_BITMASK_OPERATORS(CommandBufferResetFlatBits);

	enum class PrimitiveTopology
	{
		POINT_LIST,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,
		LINE_LIST_WITH_ADJACENCY,
		LINE_STRIP_WITH_ADJACENCY,
		TRIANGLE_LIST_WITH_ADJACENCY,
		TRIANGLE_STRIP_WITH_ADJACENCY,
		PATCH_LIST,
		Num
	};
	enum class PolygonMode
	{
		FILL,
		LINE,
		POINT,
	};
	enum class CullMode
	{
		NONE,
		FRONT,
		BACK,
		FRONT_AND_BACK,
	};
	enum class FrontFace
	{
		COUNTER_CLOCKWISE,
		CLOCKWISE,
	};
	enum class StencilOp
	{
		KEEP,
		ZERO,
		REPLACE,
		INCREMENT_AND_CLAMP,
		DECREMENT_AND_CLAMP,
		INVERT,
		INCREMENT_AND_WRAP,
		DECREMENT_AND_WRAP,
	};
	enum class StencilFaceFlagBits
	{
		FRONT_BIT = 0x00000001,
		BACK_BIT = 0x00000002,
		FRONT_AND_BACK = 0x00000003,
	};
	ENABLE_BITMASK_OPERATORS(StencilFaceFlagBits);

	enum class LogicOp
	{
		CLEAR,
		AND,
		AND_REVERSE,
		COPY,
		AND_INVERTED,
		NO_OP,
		XOR,
		OR,
		NOR,
		EQUIVALENT,
		INVERT,
		OR_REVERSE,
		COPY_INVERTED,
		OR_INVERTED,
		NAND,
		SET,
	};
	enum class BlendFactor
	{
		ZERO,
		ONE,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		SRC_ALPHA,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA,
	};
	enum class BlendOp
	{
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX,
	};
	enum class ColorComponentFlagBits
	{
		R_BIT = 0x1,
		G_BIT = 0x2,
		B_BIT = 0x4,
		A_BIT = 0x8,
	};
	ENABLE_BITMASK_OPERATORS(ColorComponentFlagBits);
	enum class DynamicState
	{
		VIEWPORT,
		SCISSOR,
		LINE_WIDTH,
		DEPTH_BIAS,
		BLEND_CONSTANTS,
		DEPTH_BOUNDS,
		STENCIL_COMPARE_MASK,
		STENCIL_WRITE_MASK,
		STENCIL_REFERENCE,
		VIEWPORT_W_SCALING,			   // Provided by VK_NV_clip_space_w_scaling
		DISCARD_RECTANGLE,			   // Provided by VK_EXT_discard_rectangles
		SAMPLE_LOCATIONS,			   // Provided by VK_EXT_sample_locations
		VIEWPORT_SHADING_RATE_PALETTE, // Provided by VK_NV_shading_rate_image
		VIEWPORT_COARSE_SAMPLE_ORDER,  // Provided by VK_NV_shading_rate_image
		EXCLUSIVE_SCISSOR,			   // Provided by VK_NV_scissor_exclusive
		LINE_STIPPLE,				   // Provided by VK_EXT_line_rasterization
		// Provided by VK_EXT_extended_dynamic_state
		CULL_MODE,
		FRONT_FACE,
		PRIMITIVE_TOPOSTLOGY,
		VIEWPORT_WITH_COUNT,
		SCISSOR_WITH_COUNT,
		VERTEX_INPUT_BINDING_STRIDE,
		DEPTH_TEST_ENABLE,
		DEPTH_WRITE_ENABLE,
		DEPTH_COMPARE_OP,
		DEPTH_BOUNDS_TEST_ENABLE,
		STENCIL_TEST_ENABLE,
		STENCIL_OP,
		Num,
	};

	enum class FenceCreateFlagBits
	{
		SIGNALED_BIT = 0x1
	};
	ENABLE_BITMASK_OPERATORS(FenceCreateFlagBits);
	enum class SemaphoreType
	{
		BINARY,
		TIMELINE,
	};
	enum class BorderColor
	{
		FLOAT_TRANSPARENT_BLACK,
		INT_TRANSPARENT_BLACK,
		FLOAT_OPAQUE_BLACK,
		INT_OPAQUE_BLACK,
		FLOAT_OPAQUE_WHITE,
		INT_OPAQUE_WHITE,
		//		FLOAT_CUSTOM_EXT,
		//		INT_CUSTOM_EXT,
	};
	// same as vulkan
	enum class PipelineStageFlagBits
	{
		TOP_OF_PIPE_BIT = 0x00000001,
		DRAW_INDIRECT_BIT = 0x00000002,
		VERTEX_INPUT_BIT = 0x00000004,
		VERTEX_SHADER_BIT = 0x00000008,
		TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
		TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
		GEOMETRY_SHADER_BIT = 0x00000040,
		FRAGMENT_SHADER_BIT = 0x00000080,
		EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
		LATE_FRAGMENT_TESTS_BIT = 0x00000200,
		COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
		COMPUTE_SHADER_BIT = 0x00000800,
		TRANSFER_BIT = 0x00001000,
		BOTTOM_OF_PIPE_BIT = 0x00002000,
		HOST_BIT = 0x00004000,
		ALL_GRAPHICS_BIT = 0x00008000,
		ALL_COMMANDS_BIT = 0x00010000,
		TRANSFORM_FEEDBACK_BIT_EXT = 0x01000000,
		CONDITIONAL_RENDERING_BIT_EXT = 0x00040000,
		RAY_TRACING_SHADER_BIT_KHR = 0x00200000,
		ACCELERATION_STRUCTURE_BUILD_BIT_KHR = 0x02000000,
		SHADING_RATE_IMAGE_BIT_NV = 0x00400000,
		TASK_SHADER_BIT_NV = 0x00080000,
		MESH_SHADER_BIT_NV = 0x00100000,
		FRAGMENT_DENSITY_PROCESS_BIT_EXT = 0x00800000,
		COMMAND_PREPROCESS_BIT_NV = 0x00020000,
	};
	ENABLE_BITMASK_OPERATORS(PipelineStageFlagBits);
	enum class DependencyFlagBits
	{
		BY_REGION_BIT = 0x00000001,
		VIEW_LOCAL_BIT = 0x00000002,
		DEVICE_GROUP_BIT = 0x00000004,
	};
	ENABLE_BITMASK_OPERATORS(DependencyFlagBits);
	enum class AccessFlagBits
	{
		INDIRECT_COMMAND_READ_BIT = 0x00000001,
		INDEX_READ_BIT = 0x00000002,
		VERTEX_ATTRIBUTE_READ_BIT = 0x00000004,
		UNIFORM_READ_BIT = 0x00000008,
		INPUT_ATTACHMENT_READ_BIT = 0x00000010,
		SHADER_READ_BIT = 0x00000020,
		SHADER_WRITE_BIT = 0x00000040,
		COLOR_ATTACHMENT_READ_BIT = 0x00000080,
		COLOR_ATTACHMENT_WRITE_BIT = 0x00000100,
		DEPTH_STENCIL_ATTACHMENT_READ_BIT = 0x00000200,
		DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000400,
		TRANSFER_READ_BIT = 0x00000800,
		TRANSFER_WRITE_BIT = 0x00001000,
		HOST_READ_BIT = 0x00002000,
		HOST_WRITE_BIT = 0x00004000,
		MEMORY_READ_BIT = 0x00008000,
		MEMORY_WRITE_BIT = 0x00010000,
		TRANSFORM_FEEDBACK_WRITE_BIT = 0x02000000,
		TRANSFORM_FEEDBACK_COUNTER_READ_BIT = 0x04000000,
		TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT = 0x08000000,
		CONDITIONAL_RENDERING_READ_BIT = 0x00100000,
		COLOR_ATTACHMENT_READ_NONCOHERENT_BIT = 0x00080000,
		ACCELERATION_STRUCTURE_READ_BIT = 0x00200000,
		ACCELERATION_STRUCTURE_WRITE_BIT = 0x00400000,
		SHADING_RATE_IMAGE_READ_BIT = 0x00800000,
		FRAGMENT_DENSITY_MAP_READ_BIT = 0x01000000,
		COMMAND_PREPROCESS_READ_BIT = 0x00020000,
		COMMAND_PREPROCESS_WRITE_BIT = 0x00040000,
	};
	ENABLE_BITMASK_OPERATORS(AccessFlagBits);

	enum class FormatFeatureFlagBits
	{
		SAMPLED_IMAGE_BIT = 0x00000001,
		STORAGE_IMAGE_BIT = 0x00000002,
		STORAGE_IMAGE_ATOMIC_BIT = 0x00000004,
		UNIFORM_TEXEL_BUFFER_BIT = 0x00000008,
		STORAGE_TEXEL_BUFFER_BIT = 0x00000010,
		STORAGE_TEXEL_BUFFER_ATOMIC_BIT = 0x00000020,
		VERTEX_BUFFER_BIT = 0x00000040,
		COLOR_ATTACHMENT_BIT = 0x00000080,
		COLOR_ATTACHMENT_BLEND_BIT = 0x00000100,
		DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000200,
		BLIT_SRC_BIT = 0x00000400,
		BLIT_DST_BIT = 0x00000800,
		SAMPLED_IMAGE_FILTER_LINEAR_BIT = 0x00001000,
		TRANSFER_SRC_BIT = 0x00004000,
		TRANSFER_DST_BIT = 0x00008000,
		MIDPOINT_CHROMA_SAMPLES_BIT = 0x00020000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT = 0x00040000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT = 0x00080000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT = 0x00100000,
		SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT = 0x00200000,
		DISJOINT_BIT = 0x00400000,
		COSITED_CHROMA_SAMPLES_BIT = 0x00800000,
		SAMPLED_IMAGE_FILTER_MINMAX_BIT = 0x00010000,
		SAMPLED_IMAGE_FILTER_CUBIC_BIT_IMG = 0x00002000,
		//#ifdef VK_ENABLE_BETA_EXTENSIONS
		//    VK_FORMAT_FEATURE_VIDEO_DECODE_OUTPUT_BIT_KHR = 0x02000000,
		//#endif
		//#ifdef VK_ENABLE_BETA_EXTENSIONS
		//    VK_FORMAT_FEATURE_VIDEO_DECODE_DPB_BIT_KHR = 0x04000000,
		//#endif
		ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR = 0x20000000,
		FRAGMENT_DENSITY_MAP_BIT_EXT = 0x01000000,
		FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x40000000,
	};
	ENABLE_BITMASK_OPERATORS(FormatFeatureFlagBits);

	enum class DebugSeverityFlagBits
	{
		INFORMATION_BIT = 0x00000001,
		WARNING_BIT = 0x00000002,
		PERFORMANCE_WARNING_BIT = 0x00000004,
		ERROR_BIT = 0x00000008,
		DEBUG_BIT = 0x00000010,
	};
	ENABLE_BITMASK_OPERATORS(DebugSeverityFlagBits);

	enum class PipelineCreateFlagBits
	{
		DISABLE_OPTIMIZATION_BIT = 0x00000001,
		ALLOW_DERIVATIVES_BIT = 0x00000002,
		DERIVATIVE_BIT = 0x00000004,
		VIEW_INDEX_FROM_DEVICE_INDEX_BIT = 0x00000008,
		DISPATCH_BASE_BIT = 0x00000010,
		RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR = 0x00004000,
		RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR = 0x00008000,
		RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR = 0x00010000,
		RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR = 0x00020000,
		RAY_TRACING_SKIP_TRIANGLES_BIT_KHR = 0x00001000,
		RAY_TRACING_SKIP_AABBS_BIT_KHR = 0x00002000,
		DEFER_COMPILE_BIT_NV = 0x00000020,
		CAPTURE_STATISTICS_BIT_KHR = 0x00000040,
		CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR = 0x00000080,
		INDIRECT_BINDABLE_BIT_NV = 0x00040000,
		LIBRARY_BIT_KHR = 0x00000800,
		FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT = 0x00000100,
		EARLY_RETURN_ON_FAILURE_BIT_EXT = 0x00000200,
	};
	ENABLE_BITMASK_OPERATORS(PipelineCreateFlagBits);

	//=============================================
	FormatAttribute GetFormatAttribute(Format format);
	uint32_t GetFormatComponentNum(Format format);
	bool GetFormatNormalized(Format format);
	uint32_t GetFormatSize(Format format);
	DataType GetFormatDataType(Format format);

	const char *GetGLSLKeywordName(GLSLKeyword keyword);
}