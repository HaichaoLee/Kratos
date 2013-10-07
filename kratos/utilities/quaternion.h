/*
==============================================================================
KratosMultiScaleApplication
A library based on:
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi, Janosch Stascheit, Felix Nagel
pooyan@cimne.upc.edu
rrossi@cimne.upc.edu
janosch.stascheit@rub.de
nagel@sd.rub.de
- CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain
- Ruhr-University Bochum, Institute for Structural Mechanics, Germany


Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNERS.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/
//
//   Project Name:        Kratos
//   Last Modified by:    $Author: Massimo Petracca $
//   Date:                $Date: 2013-09-18 22:43:00 $
//   Revision:            $Revision: 1.00 $
//
//

#if !defined(QUATERNION_H_INCLUDED)
#define QUATERNION_H_INCLUDED


namespace Kratos
{

	template<class T>
	class Quaternion
	{

	public:
		
		/**
		Creates a Zero Quaternion.
		*/
		Quaternion()
			: mX(0.0)
			, mY(0.0)
			, mZ(0.0)
			, mW(0.0)
		{
		}

		/**
		Creates a Quaternion from its coefficients.
		*/
		Quaternion(T w, T x, T y, T z)
			: mX(x)
			, mY(y)
			, mZ(z)
			, mW(w)
		{
		}
		
		/**
		Creates a Quaternion from another Quaternion.
		*/
		Quaternion(const Quaternion& other)
			: mX(other.mX)
			, mY(other.mY)
			, mZ(other.mZ)
			, mW(other.mW)
		{
		}

	public:
		
		Quaternion& operator= (const Quaternion& other)
		{
			if(this != &other) {
				mX = other.mX;
				mY = other.mY;
				mZ = other.mZ;
				mW = other.mW;
			}
			return *this;
		}
		
	public:

		/**
		Returns the X coefficient of this quaternion.
		*/
		inline const T x()const { return mX; }
		
		/**
		Returns the Y coefficient of this quaternion.
		*/
		inline const T y()const { return mY; }
		
		/**
		Returns the Z coefficient of this quaternion.
		*/
		inline const T z()const { return mZ; }
		
		/**
		Returns the W coefficient of this quaternion.
		*/
		inline const T w()const { return mW; }
		
	public:

		/**
		Returns the squared norm of this quaternion.
		x*x + y*y + z*z + w*w
		*/
		inline const T squaredNorm()const
		{
			return mX*mX + mY*mY + mZ*mZ + mW*mW;
		}
		
		/**
		Returns the norm of this quaternion.
		sqrt(x*x + y*y + z*z + w*w)
		*/
		inline const T norm()const
		{
			return std::sqrt( this->squaredNorm() );
		}
		
		/**
		Makes this Quaternion a Unit Quaternion.
		If this Quaternion is already normalized this is a no-op
		*/
		inline void normalize()
		{
			T n = this->squaredNorm();
			if(n > 0.0 && n != 1.0) {
				n = std::sqrt(n);
				mX /= n;
				mY /= n;
				mZ /= n;
				mW /= n;
			}
		}
		
		/**
	    Returns the Conjugate of this Quaternion, which represents the opposite rotation
		*/
		inline Quaternion conjugate()const
		{
			return Quaternion(mW, -mX, -mY, -mZ);
		}
		
		/**
		Constructs a Rotation Matrix from this Quaternion.
		The rotation matrix type is the template argument, no check is made on the type of
		this matrix.
		These assumptions are made:
		The matrix should provide an indexed access like m(i, j) where i and j are indices
		from 0 to 2.
		This means that the input matrix is a C-Style 3x3 Matrix.
		All the 9 coefficients are properly set so there's no need to set the matrix to Zero
		before calling this function.
		*/
		template<class TMatrix3x3>
		inline void ToRotationMatrix(TMatrix3x3& R)const
		{
			R(0, 0) = 2.0 * ( mW*mW + mX*mX - 0.5 );
			R(0, 1) = 2.0 * ( mX*mY - mW*mZ );
			R(0, 2) = 2.0 * ( mX*mZ + mW*mY );

			R(1, 0) = 2.0 * ( mY*mX + mW*mZ );
			R(1, 1) = 2.0 * ( mW*mW + mY*mY - 0.5 );
			R(1, 2) = 2.0 * ( mY*mZ - mX*mW );

			R(2, 0) = 2.0 * ( mZ*mX - mW*mY );
			R(2, 1) = 2.0 * ( mZ*mY + mW*mX );
			R(2, 2) = 2.0 * ( mW*mW + mZ*mZ - 0.5 );
		}
		
		/**
		Extracts the Rotation Vector (Euler Angles) from this Quaternion
		*/
		inline void ToRotationVector(T& rx, T& ry, T& rz)const
		{
			T xx, yy, zz, ww;
			
			if(mW < 0.0) {
				xx = -mX;
				yy = -mY;
				zz = -mZ;
				ww = -mW;
			}
			else {
				xx = mX;
				yy = mY;
				zz = mZ;
				ww = mW;
			}

			T vNorm = xx*xx + yy*yy + zz*zz;
			if(vNorm == 0.0) {
				rx = 0.0;
				ry = 0.0;
				rz = 0.0;
				return;
			}

			if(vNorm != 1.0)
				vNorm = std::sqrt(vNorm);

			T mult = (vNorm < ww) ? (2.0 / vNorm * std::asin(vNorm)) : (2.0 / vNorm * std::acos(ww));

			rx = xx * mult;
			ry = yy * mult;
			rz = zz * mult;
		}
		
		/**
		Extracts the Rotation Vector (Euler Angles) from this Quaternion
		The vector type is the template parameter. No check is made on this type.
		The following assumptions are made:
		The vector type should provide indexing like vector(i) where i goes from 0 to 2.
		(i.e. a C-Style vector of size 3)
		*/
		template<class TVector3>
		inline void ToRotationVector(TVector3& v)const
		{
			this->ToRotationVector(v(0), v(1), v(2));
		}

		/**
		Rotates a vector using this quaternion.
		Note: this is faster than constructing the rotation matrix and perform the matrix
		multiplication for a single vector.
		The vector type is the template parameter. No check is made on this type.
		The following assumptions are made:
		The vector type should provide indexing like vector(i) where i goes from 0 to 2.
		(i.e. a C-Style vector of size 3)
		*/
		template<class TVector3_A, class TVector3_B>
		inline void RotateVector3(const TVector3_A& a, TVector3_B& b)const
		{
			// b = 2.0 * cross( this->VectorialPart, a )
			b(0) = 2.0 * (mY * a(2) - mZ * a(1));
			b(1) = 2.0 * (mZ * a(0) - mX * a(2));
			b(2) = 2.0 * (mX * a(1) - mY * a(0));

			// c = cross( this->VectorialPart, b )
			T c0 = mY * a(2) - mZ * a(1);
			T c1 = mZ * a(0) - mX * a(2);
			T c2 = mX * a(1) - mY * a(0);

			// set results
			b(0) = a(0) + b(0)*mW + c0;
			b(1) = a(1) + b(1)*mW + c1;
			b(2) = a(2) + b(2)*mW + c2;
		}

	public:
		
		/**
		Returns the Identity Quaternion (i.e. a Quaternion that represents a Zero rotation)
		*/
		static inline Quaternion Identity()
		{
			return Quaternion(1.0, 0.0, 0.0, 0.0);
		}
		
		/**
		Returns a Quaternion that represents a rotation of an angle 'radians' around the axis (x, y, z)
		*/
		static inline Quaternion FromAxisAngle(T x, T y, T z, T radians)
		{
			T sqnorm = x*x + y*y + z*z;
			if (sqnorm == 0.0)
				return Quaternion::Identity();

			if(sqnorm != 1.0) {
				T norm = std::sqrt(sqnorm);
				x /= norm;
				y /= norm;
				z /= norm;
			}
			
			T halfAngle = radians * 0.5;

			T s = std::sin(halfAngle);
			T q0 = std::cos(halfAngle);

			Quaternion result(q0, s*x, s*y, s*z);
			result.normalize();

			return result;
		}

		/**
		Returns a Quaternion from 3 Euler Angles
		*/
		static inline Quaternion FromRotationVector(T rx, T ry, T rz)
		{
			T rModulus = rx*rx + ry*ry + rz*rz;
			if(rModulus == 0.0)
				return Quaternion::Identity();

			if(rModulus != 1.0) {
				rModulus = std::sqrt(rModulus);
				rx /= rModulus;
				ry /= rModulus;
				rz /= rModulus;
			}

			T halfAngle = rModulus * 0.5;

			T q0 = std::cos(halfAngle);
			T s = std::sin(halfAngle);

			return Quaternion(q0, rx*s, ry*s, rz*s);
		}
		
		/**
		Returns a Quaternion from 3 Euler Angles.
		The vector type is the template parameter. No check is made on this type.
		The following assumptions are made:
		The vector type should provide indexing like vector(i) where i goes from 0 to 2.
		(i.e. a C-Style vector of size 3)
		*/
		template<class TVector3>
		static inline Quaternion FromRotationVector(const TVector3& v)
		{
			return Quaternion::FromRotationVector(v(0), v(1), v(2));
		}

		/**
		Returns a Quaternion from a Rotation Matrix.
		The rotation matrix type is the template argument, no check is made on the type of
		this matrix.
		These assumptions are made:
		The matrix should provide an indexed access like m(i, j) where i and j are indices
		from 0 to 2.
		This means that the input matrix is a C-Style 3x3 Matrix.
		*/
		template<class TMatrix3x3>
		static inline Quaternion FromRotationMatrix(const TMatrix3x3 & m)
		{
			T xx = m(0, 0);
			T yy = m(1, 1);
			T zz = m(2, 2);
			T tr = xx + yy + zz;
			if (tr > 0.0)
			{
				T S = std::sqrt(tr + 1.0) * 2.0;
				return Quaternion(
					0.25 * S,
					(m(2, 1) - m(1, 2)) / S,
					(m(0, 2) - m(2, 0)) / S,
					(m(1, 0) - m(0, 1)) / S
					);
			}
			else if ((xx > yy) & (xx > zz))
			{
				T S = std::sqrt(1.0 + xx - yy - zz) * 2.0;
				return Quaternion(
					(m(2, 1) - m(1, 2)) / S,
					0.25 * S,
					(m(0, 1) + m(1, 0)) / S,
					(m(0, 2) + m(2, 0)) / S
					);
			}
			else if (yy > zz)
			{
				T S = std::sqrt(1.0 + yy - xx - zz) * 2.0;
				return Quaternion(
					(m(0, 2) - m(2, 0)) / S,
					(m(0, 1) + m(1, 0)) / S,
					0.25 * S,
					(m(1, 2) + m(2, 1)) / S
					);
			}
			else
			{
				T S = std::sqrt(1.0 + zz - xx - yy) * 2.0;
				return Quaternion(
					(m(1, 0) - m(0, 1)) / S,
					(m(0, 2) + m(2, 0)) / S,
					(m(1, 2) + m(2, 1)) / S,
					0.25 * S
					);
			}
		}
		
	private:

		T mX;
		T mY;
		T mZ;
		T mW;

	};

	/**
	Performs a Quaternion-Quaternion product and returns the concatenation
	of the two input quaternions (i.e. the compound rotation)
	*/
	template<class T>
	inline Quaternion<T> operator* (const Quaternion<T>& a, const Quaternion<T>& b)
	{
		return Quaternion<T>(
			a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z(),
			a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y(),
			a.w() * b.y() + a.y() * b.w() + a.z() * b.x() - a.x() * b.z(),
			a.w() * b.z() + a.z() * b.w() + a.x() * b.y() - a.y() * b.x()
		);
	}

} // namespace Kratos

#endif // QUATERNION_H_INCLUDED
