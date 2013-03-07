/*
 * misc helper
 * @author tweber
 * @date 07-mar-2013
 */

#ifndef __MIEZE_MISC_HELPER__
#define __MIEZE_MISC_HELPER__

// deletes an object when going out of scope
template<class T> class autodeleter
{
protected:
        T *m_t;
        bool m_bIsArray;

public:
        autodeleter(T* t, bool bIsArray=false) : m_t(t), m_bIsArray(bIsArray)
        {}

        ~autodeleter()
        {
                if(m_t)
                {
                        if(m_bIsArray)
                                delete[] m_t;
                        else
                                delete m_t;
                        m_t = 0;
                }
        }
};

template<typename T> T safe_log10(T t)
{
        const T t_invalid = T(-10);

        if(t > T(0))
                return log10(t);

        return t_invalid;
}

// pixel -> val
inline double tic_trafo(unsigned int iDim, double dMin, double dMax, bool bLog, double dPix)
{
        if(bLog)
        {
                dMin = safe_log10(dMin);
                dMax = safe_log10(dMax);
        }

        double dval = dMin + dPix/double(iDim) * (dMax-dMin);
        if(bLog)
                dval = pow(10., dval);

        return dval;
}

// val -> pixel
inline double tic_trafo_inv(unsigned int iDim, double dMin, double dMax, bool bLog, double dVal)
{
        if(bLog)
        {
                dMin = safe_log10(dMin);
                dMax = safe_log10(dMax);

                dVal = safe_log10(dVal);
        }

        double dpix = (dVal-dMin)/(dMax-dMin) * double(iDim);
        return dpix;
}

#endif
