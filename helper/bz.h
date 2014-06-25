/*
 * Calculation of first Brillouin zone
 * @author Tobias Weber
 * @date jun-2014
 */

#ifndef __BZ_H__
#define __BZ_H__

#include "exception.h"
#include "math.h"
#include "geo.h"
#include <vector>


template<typename T=double>
class Brillouin2D
{
	public:
		template<typename _T> using t_vecpair = std::pair<ublas::vector<_T>, ublas::vector<_T> >;
		template<typename _T> using t_vertices = std::vector<t_vecpair<_T> >;

	protected:
		ublas::vector<T> m_vecCentralReflex;
		std::vector<ublas::vector<T> > m_vecNeighbours;
		std::vector<t_vecpair<T> > m_vecVertices;
		bool m_bValid = 1;

		const T eps = 0.001;

	protected:
		const t_vecpair<T>* GetNextVertexPair(const t_vertices<T>& vecPts, unsigned int *pIdx=0)
		{
			const ublas::vector<T>& vecLast = m_vecVertices.rbegin()->second;
			for(unsigned int iPt=0; iPt<vecPts.size(); ++iPt)
			{
				const t_vecpair<T>& vecp = vecPts[iPt];

				if(vec_equal(vecp.first, vecLast, eps))
				{
					if(pIdx) *pIdx = iPt;
					return &vecp;
				}
			}

			if(vecPts.size())
			{
				std::cerr << "Error: Invalid vertices in Brillouin zone." << std::endl;
				m_bValid = 0;
			}
			return 0;
		}

	public:
		Brillouin2D()
		{}
		virtual ~Brillouin2D()
		{}

		const t_vertices<T>& GetVertices() const { return m_vecVertices; }

		void Clear()
		{
			m_vecNeighbours.clear();
			m_vecVertices.clear();
		}

		bool IsValid() const { return m_bValid; }

		void SetCentralReflex(const ublas::vector<T>& vec)
		{
			if(vec.size() != 2)
				throw Err("Brillouin2D needs 2d vectors.");

			m_vecCentralReflex = vec;
		}
		void AddReflex(const ublas::vector<T>& vec)
		{
			if(vec.size() != 2)
				throw Err("Brillouin2D needs 2d vectors.");

			m_vecNeighbours.push_back(vec);
		}

		void CalcBZ()
		{
			m_bValid = 1;

			// calculate perpendicular lines
			std::vector<Line<T> > vecMiddlePerps;
			vecMiddlePerps.reserve(m_vecNeighbours.size());

			t_vertices<T> vecPts;

			for(const ublas::vector<T>& vecN : m_vecNeighbours)
			{
				Line<T> line(m_vecCentralReflex, vecN-m_vecCentralReflex);
				Line<T> lineperp;
				if(!line.GetMiddlePerp(lineperp))
					continue;

				vecMiddlePerps.push_back(lineperp);
			}


			// calculate intersections
			for(unsigned int iThisLine=0; iThisLine<vecMiddlePerps.size(); ++iThisLine)
			{
				const Line<T>& lineThis = vecMiddlePerps[iThisLine];
				T tPos = std::numeric_limits<T>::max();
				T tNeg = -tPos;

				for(unsigned int iOtherLine=0; iOtherLine<vecMiddlePerps.size(); ++iOtherLine)
				{
					if(iThisLine == iOtherLine)
						continue;

					T t;
					if(!lineThis.intersect(vecMiddlePerps[iOtherLine], t))
						continue;

					if(t>0.)
					{
						if(t < tPos)
							tPos = t;
					}
					else
					{
						if(t > tNeg)
							tNeg = t;
					}
				}

				ublas::vector<T> vecUpper = lineThis(tPos);
				ublas::vector<T> vecLower = lineThis(tNeg);
				if(vec_equal(vecUpper, vecLower, eps))
					continue;

				vecPts.push_back(t_vecpair<T>(vecUpper, vecLower));
			}


			// remove unnecessary vertices
			for(const Line<T>& line : vecMiddlePerps)
			{
				bool bSideReflex = line.GetSide(m_vecCentralReflex);

				for(unsigned int iPt=0; iPt<vecPts.size(); ++iPt)
				{
					ublas::vector<T>& vecUpper = vecPts[iPt].first;
					ublas::vector<T>& vecLower = vecPts[iPt].second;

					T tDistUpper, tDistLower;
					bool bSideUpper = line.GetSide(vecUpper, &tDistUpper);
					bool bSideLower = line.GetSide(vecLower, &tDistLower);

					if((bSideUpper!=bSideReflex && tDistUpper>eps) ||
							(bSideLower!=bSideReflex && tDistLower>eps))
					{
						//std::cout << "Erasing " << iPt << std::endl;
						vecPts.erase(vecPts.begin()+iPt);
						--iPt;
						continue;
					}
				}
			}


			// sort vertices
			m_vecVertices.clear();
			m_vecVertices.reserve(vecPts.size());
			m_vecVertices.push_back(*vecPts.begin());
			vecPts.erase(vecPts.begin());

			unsigned int iIdx;
			while(const t_vecpair<T>* pPair = GetNextVertexPair(vecPts, &iIdx))
			{
				m_vecVertices.push_back(*pPair);
				vecPts.erase(vecPts.begin()+iIdx);
			}


			//for(const t_vecpair<T>& vecPair : m_vecVertices)
			//	std::cout << vecPair.first << ", " << vecPair.second << std::endl;
		}
};

#endif
