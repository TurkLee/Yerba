#pragma once

#include <iostream>

struct Color
{
	unsigned char red, green, blue, alpha;

	constexpr Color() : red(0), green(0), blue(0), alpha(255) { }

	constexpr Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
		: red{ r }, green{ g }, blue{ b }, alpha{ a } { }

	unsigned long hextoRGBA(int r, int g, int b, int a)
	{
		return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8)
			+ (a & 0xff);
	}

	constexpr Color& operator *=(const float coeff)
	{
		this->red = static_cast<int>(this->red * coeff);
		this->green = static_cast<int>(this->green * coeff);
		this->blue = static_cast<int>(this->blue * coeff);
		return *this;
	}

	constexpr Color operator ()(const int a) const
	{
		return Color(red, green, blue, a);
	}

	constexpr Color& operator /=(const float div)
	{
		const auto flDiv = 1.f / div;
		*this *= flDiv;
		return *this;
	}

	constexpr Color& operator *(const float coeff) const
	{
		auto color = *this;
		return color *= coeff;
	}

	//constexpr DWORD GetARGB() const
	//{
	//	return 0;
	//}

	constexpr Color& FromHSV(float h, float s, float v)
	{
		float colOut[3]{ };
		if (s == 0.0f)
		{
			red = green = blue = static_cast<int>(v * 255);
			return *this;
		}

		h = std::fmodf(h, 1.0f) / (60.0f / 360.0f);
		int   i = static_cast<int>(h);
		float f = h - static_cast<float>(i);
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * f);
		float t = v * (1.0f - s * (1.0f - f));

		switch (i)
		{
		case 0:
			colOut[0] = v;
			colOut[1] = t;
			colOut[2] = p;
			break;
		case 1:
			colOut[0] = q;
			colOut[1] = v;
			colOut[2] = p;
			break;
		case 2:
			colOut[0] = p;
			colOut[1] = v;
			colOut[2] = t;
			break;
		case 3:
			colOut[0] = p;
			colOut[1] = q;
			colOut[2] = v;
			break;
		case 4:
			colOut[0] = t;
			colOut[1] = p;
			colOut[2] = v;
			break;
		case 5: default:
			colOut[0] = v;
			colOut[1] = p;
			colOut[2] = q;
			break;
		}

		red = static_cast<int>(colOut[0] * 255);
		green = static_cast<int>(colOut[1] * 255);
		blue = static_cast<int>(colOut[2] * 255);
		return *this;
	}

	inline int r() const { return red; }
	inline int g() const { return green; }
	inline int b() const { return blue; }
	inline int a() const { return alpha; }

	constexpr auto ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
	{
		if (s == 0.0f)
		{
			// gray
			out_r = out_g = out_b = v;
			return;
		}

		h = fmodf(h, 1.0f) / (60.0f / 360.0f);
		int   i = (int)h;
		float f = h - (float)i;
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * f);
		float t = v * (1.0f - s * (1.0f - f));

		switch (i)
		{
		case 0: out_r = v; out_g = t; out_b = p; break;
		case 1: out_r = q; out_g = v; out_b = p; break;
		case 2: out_r = p; out_g = v; out_b = t; break;
		case 3: out_r = p; out_g = q; out_b = v; break;
		case 4: out_r = t; out_g = p; out_b = v; break;
		case 5: default: out_r = v; out_g = p; out_b = q; break;
		}
	}

	constexpr auto ToHSV(float& h, float& s, float& v)
	{
		float col[3] = { red / 255.f, green / 255.f, blue / 255.f };

		float K = 0.f;
		if (col[1] < col[2])
		{
			swap(col[1], col[2]);
			K = -1.f;
		}
		if (col[0] < col[1])
		{
			swap(col[0], col[1]);
			K = -2.f / 6.f - K;
		}

		const float chroma = col[0] - (col[1] < col[2] ? col[1] : col[2]);
		h = colfabs(K + (col[1] - col[2]) / (6.f * chroma + 1e-20f));
		s = chroma / (col[0] + 1e-20f);
		v = col[1];
	}

	template <typename T>
	Color hsv_to_rgb(T h, T s, T v) const
	{
		int _r, _g, _b;

		if (s != 0)
		{
			int i;
			int f, p, q, t;

			h == 360 ? h = 0 : h = h / 60;
			i = static_cast<int>(trunc(h));
			f = h - i;

			p = v * (1 - s);
			q = v * (1 - s * f);
			t = v * (1 - s * (1 - f));

			switch (i)
			{
			case 0:
				_r = v;
				_g = t;
				_b = p;
				break;

			case 1:
				_r = q;
				_g = v;
				_b = p;
				break;

			case 2:
				_r = p;
				_g = v;
				_b = t;
				break;

			case 3:
				_r = p;
				_g = q;
				_b = v;
				break;

			case 4:
				_r = t;
				_g = p;
				_b = v;
				break;

			default:
				_r = v;
				_g = p;
				_b = q;
				break;
			}
		}
		else
		{
			_r = v;
			_g = v;
			_b = v;
		}

		return Color(static_cast<uint8_t>(_r * 255), static_cast<uint8_t>(_g * 255), static_cast<uint8_t>(_b * 255));
	}

	static Color FromHSB(float hue, float saturation, float brightness)
	{
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}
		else if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	static constexpr Color Black(unsigned char a = 255) { return { 0, 0, 0, a }; }
	static constexpr Color Grey(unsigned char  a = 255) { return { 127, 127, 127, a }; }
	static constexpr Color White(unsigned char a = 255) { return { 255, 255, 255, a }; }
	static constexpr Color Red(unsigned char   a = 255) { return { 255, 0, 0, a }; }
	static constexpr Color Green(unsigned char a = 255) { return { 0, 255, 0, a }; }
	static constexpr Color Blue(unsigned char  a = 255) { return { 0, 0, 255, a }; }

private:
	constexpr void  swap(float& a, float& b) { float tmp = a; a = b; b = tmp; }
	constexpr float colfabs(const float& x) { return x < 0 ? x * -1 : x; }

};