#pragma once

#include <vector>
#include <limits>
#include <stdexcept>

namespace Pathfinding
{
	typedef D3DXVECTOR2 Vector2;
	typedef D3DXVECTOR3 Vector3;
	typedef D3DXVECTOR4 Vector4;
	typedef D3DXMATRIX Mat4x4;
	typedef D3DXCOLOR Color;

	struct PointId
	{
		int value;

		PointId() : value(-1) {}
		explicit PointId(int v) : value(v) {}

		bool IsValid() const { return value >= 0; }

		bool operator==(PointId other) const { return value == other.value; }
		bool operator!=(PointId other) const { return value != other.value; }
		bool operator<(PointId other) const { return value < other.value; }
		bool operator>(PointId other) const { return value > other.value; }

		operator int() const { return value; }
	};

	struct TriangleId
	{
		int value;

		TriangleId() : value(-1) {}
		explicit TriangleId(int v) : value(v) {}

		bool IsValid() const { return value >= 0; }

		bool operator==(TriangleId other) const { return value == other.value; }
		bool operator!=(TriangleId other) const { return value != other.value; }
		bool operator<(TriangleId other) const { return value < other.value; }
		bool operator>(TriangleId other) const { return value > other.value; }

		operator int() const { return value; }
	};

	// Common color definitions
	inline const Color COLOR_TRANSPARENT = Color(1.0f, 0.0f, 1.0f, 0.0f);
	inline const Color COLOR_WHITE = Color(1.0f, 1.0f, 1.0f, 1.0f);
	inline const Color COLOR_BLACK = Color(0.0f, 0.0f, 0.0f, 1.0f);
	inline const Color COLOR_RED = Color(1.0f, 0.0f, 0.0f, 1.0f);
	inline const Color COLOR_GREEN = Color(0.0f, 1.0f, 0.0f, 1.0f);
	inline const Color COLOR_BLUE = Color(0.0f, 0.0f, 1.0f, 1.0f);
	inline const Color COLOR_YELLOW = Color(1.0f, 1.0f, 0.0f, 1.0f);
	inline const Color COLOR_CYAN = Color(0.0f, 1.0f, 1.0f, 1.0f);
	inline const Color COLOR_MAGENTA = Color(1.0f, 0.0f, 1.0f, 1.0f);
	inline const Color COLOR_GRAY = Color(0.5f, 0.5f, 0.5f, 1.0f);

	inline Color WithAlpha(const Color& base, float alpha)
	{
		return Color(base.r, base.g, base.b, alpha);
	}

	inline const Vector2 VECTOR2_NAN = Vector2(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());

	template<typename T>
	class Grid2D
	{
	public:
		Grid2D(size_t rows, size_t cols, const T& value = T())
			: m_rows(rows), m_cols(cols), m_data(rows, std::vector<T>(cols, value))
		{
		}

		// Access element at (row, col)
		T& operator()(size_t row, size_t col)
		{
			if (row >= m_rows || col >= m_cols)
				throw std::out_of_range("Grid2D index out of range");
			return m_data[row][col];
		}

		const T& operator()(size_t row, size_t col) const
		{
			if (row >= m_rows || col >= m_cols)
				throw std::out_of_range("Grid2D index out of range");
			return m_data[row][col];
		}

		// Row access
		std::vector<T>& operator[](size_t row) {
			if (row >= m_rows)
				throw std::out_of_range("Grid2D row index out of range");
			return m_data[row];
		}

		// Row access
		const std::vector<T>& operator[](size_t row) const {
			if (row >= m_rows)
				throw std::out_of_range("Grid2D row index out of range");
			return m_data[row];
		}

		void Fill(const T& value)
		{
			for (size_t row = 0; row < m_rows; ++row)
			{
				std::fill(m_data[row].begin(), m_data[row].end(), value);
			}
		}

		size_t GetRowCount() const { return m_rows; }
		size_t GetColCount() const { return m_cols; }

	private:
		size_t m_rows;
		size_t m_cols;
		std::vector<std::vector<T>> m_data;
	};
}