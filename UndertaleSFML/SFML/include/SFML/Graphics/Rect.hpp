////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_RECT_HPP
#define SFML_RECT_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Vector2.hpp>
#include <algorithm>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Utility class for manipulating 2D axis aligned rectangles
///
////////////////////////////////////////////////////////////
template <typename T>
class Rect
{
public:

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates an empty rectangle (it is equivalent to calling
    /// Rect(0, 0, 0, 0)).
    ///
    ////////////////////////////////////////////////////////////
	Rect() :left(T{}),top(T{}),width(T{}),height(T{}){ }





    ////////////////////////////////////////////////////////////
    /// \brief Construct the rectangle from its coordinates
    ///
    /// Be careful, the last two parameters are the width
    /// and height, not the right and bottom coordinates!
    ///
    /// \param rectLeft   Left coordinate of the rectangle
    /// \param rectTop    Top coordinate of the rectangle
    /// \param rectWidth  Width of the rectangle
    /// \param rectHeight Height of the rectangle
    ///
    ////////////////////////////////////////////////////////////
	template<typename T1,typename T2, typename T3, typename T4>
	Rect(T1 rectLeft, T2 rectTop, T3 rectWidth, T4 rectHeight) : left(static_cast<T>(rectLeft)),top(static_cast<T>(rectTop)),width(static_cast<T>(rectWidth)),height(static_cast<T>(rectHeight)){}

	// just a sized rect with 0 top/left
	template<typename T1, typename T2>
	Rect(T1 rectWidth, T2 rectHeight) : left(T{}), top(T{}), width(static_cast<T>(rectWidth)), height(static_cast<T>(rectHeight)) {}

	template<typename T1>
	Rect(const Vector2<T1>& size) : left(T{}), top(T{}), width(static_cast<T>(size.x)), height(static_cast<T>(size.y)) {}
    ////////////////////////////////////////////////////////////
    /// \brief Construct the rectangle from position and size
    ///
    /// Be careful, the last parameter is the size,
    /// not the bottom-right corner!
    ///
    /// \param position Position of the top-left corner of the rectangle
    /// \param size     Size of the rectangle
    ///
    ////////////////////////////////////////////////////////////
	template<typename T1, typename T2>
    Rect(const Vector2<T1>& position, const Vector2<T2>& size) : left(static_cast<T>(position.x)), top(static_cast<T>(position.y)), width(static_cast<T>(size.x)), height(static_cast<T>(size.y)){}

    ////////////////////////////////////////////////////////////
    /// \brief Construct the rectangle from another type of rectangle
    ///
    /// This constructor doesn't replace the copy constructor,
    /// it's called only when U != T.
    /// A call to this constructor will fail to compile if U
    /// is not convertible to T.
    ///
    /// \param rectangle Rectangle to convert
    ///
    ////////////////////////////////////////////////////////////
	template <typename U>
	explicit Rect(const Rect<U>& rectangle) :
		left(static_cast<T>(rectangle.left)),
		top(static_cast<T>(rectangle.top)),
		width(static_cast<T>(rectangle.width)),
		height(static_cast<T>(rectangle.height))
	{
	}
	////////////////////////////////////////////////////////////
	template <typename R>
	inline bool contains(R x, R y) const
	{
		// Rectangles with negative dimensions are allowed, so we must handle them correctly

		// Compute the real min and max of the rectangle on both axes
		T minX = std::min(left, static_cast<T>(left + width));
		T maxX = std::max(left, static_cast<T>(left + width));
		T minY = std::min(top, static_cast<T>(top + height));
		T maxY = std::max(top, static_cast<T>(top + height));

		return (x >= minX) && (x < maxX) && (y >= minY) && (y < maxY);
	}
	////////////////////////////////////////////////////////////
	template <typename R>
	bool contains(const Vector2<R>& point) const
	{
		return contains(point.x, point.y);
	}

    ////////////////////////////////////////////////////////////
    /// \brief Check the intersection between two rectangles
    ///
    /// \param rectangle Rectangle to test
    ///
    /// \return True if rectangles overlap, false otherwise
    ///
    /// \see contains
    ///
    ////////////////////////////////////////////////////////////
    bool intersects(const Rect<T>& rectangle) const;

    ////////////////////////////////////////////////////////////
    /// \brief Check the intersection between two rectangles
    ///
    /// This overload returns the overlapped rectangle in the
    /// \a intersection parameter.
    ///
    /// \param rectangle    Rectangle to test
    /// \param intersection Rectangle to be filled with the intersection
    ///
    /// \return True if rectangles overlap, false otherwise
    ///
    /// \see contains
    ///
    ////////////////////////////////////////////////////////////
    bool intersects(const Rect<T>& rectangle, Rect<T>& intersection) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
	// bit hacky but hey it works
	sf::Vector2<T>& position() { return *reinterpret_cast<sf::Vector2<T>*>(&width); }
	sf::Vector2<T>& size() { return *reinterpret_cast<sf::Vector2<T>*>(&width); } // offset
	const sf::Vector2<T>& position() const { return *reinterpret_cast<const sf::Vector2<T>*>(&left); }
	const sf::Vector2<T>& size() const { return *reinterpret_cast<const sf::Vector2<T>*>(&width); } // offset
    T left;   ///< Left coordinate of the rectangle
    T top;    ///< Top coordinate of the rectangle
    T width;  ///< Width of the rectangle
    T height; ///< Height of the rectangle
};

////////////////////////////////////////////////////////////
/// \relates Rect
/// \brief Overload of binary operator ==
///
/// This operator compares strict equality between two rectangles.
///
/// \param left  Left operand (a rectangle)
/// \param right Right operand (a rectangle)
///
/// \return True if \a left is equal to \a right
///
////////////////////////////////////////////////////////////
template <typename T>
bool operator ==(const Rect<T>& left, const Rect<T>& right);

////////////////////////////////////////////////////////////
/// \relates Rect
/// \brief Overload of binary operator !=
///
/// This operator compares strict difference between two rectangles.
///
/// \param left  Left operand (a rectangle)
/// \param right Right operand (a rectangle)
///
/// \return True if \a left is not equal to \a right
///
////////////////////////////////////////////////////////////
template <typename T>
bool operator !=(const Rect<T>& left, const Rect<T>& right);

#include <SFML/Graphics/Rect.inl>

// Create typedefs for the most common types
//using IntRect = Rect<int>;
//using FloatRect = Rect<float>;
typedef typename Rect<int>   IntRect;
typedef typename Rect<float> FloatRect;

} // namespace sf


#endif // SFML_RECT_HPP


////////////////////////////////////////////////////////////
/// \class sf::Rect
/// \ingroup graphics
///
/// A rectangle is defined by its top-left corner and its size.
/// It is a very simple class defined for convenience, so
/// its member variables (left, top, width and height) are public
/// and can be accessed directly, just like the vector classes
/// (Vector2 and Vector3).
///
/// To keep things simple, sf::Rect doesn't define
/// functions to emulate the properties that are not directly
/// members (such as right, bottom, center, etc.), it rather
/// only provides intersection functions.
///
/// sf::Rect uses the usual rules for its boundaries:
/// \li The left and top edges are included in the rectangle's area
/// \li The right (left + width) and bottom (top + height) edges are excluded from the rectangle's area
///
/// This means that sf::IntRect(0, 0, 1, 1) and sf::IntRect(1, 1, 1, 1)
/// don't intersect.
///
/// sf::Rect is a template and may be used with any numeric type, but
/// for simplicity the instantiations used by SFML are typedef'd:
/// \li sf::Rect<int> is sf::IntRect
/// \li sf::Rect<float> is sf::FloatRect
///
/// So that you don't have to care about the template syntax.
///
/// Usage example:
/// \code
/// // Define a rectangle, located at (0, 0) with a size of 20x5
/// sf::IntRect r1(0, 0, 20, 5);
///
/// // Define another rectangle, located at (4, 2) with a size of 18x10
/// sf::Vector2i position(4, 2);
/// sf::Vector2i size(18, 10);
/// sf::IntRect r2(position, size);
///
/// // Test intersections with the point (3, 1)
/// bool b1 = r1.contains(3, 1); // true
/// bool b2 = r2.contains(3, 1); // false
///
/// // Test the intersection between r1 and r2
/// sf::IntRect result;
/// bool b3 = r1.intersects(r2, result); // true
/// // result == (4, 2, 16, 3)
/// \endcode
///
////////////////////////////////////////////////////////////
