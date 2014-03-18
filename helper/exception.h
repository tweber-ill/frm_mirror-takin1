/*
 * Exception
 * @author tweber
 * @date 04-mar-2014
 */

#ifndef __MY_EXCEPT_H__
#define __MY_EXCEPT_H__

#include <exception>
#include <string>

class Err : public std::exception
{
	protected:
		std::string m_strErr;

	public:
		Err(const std::string& strErr) noexcept
				: m_strErr("Error: " + strErr)
		{}

		Err(const char* pcErr) noexcept : Err(std::string(pcErr))
		{}

		virtual ~Err() noexcept
		{}

		virtual const char* what() const noexcept
		{
			return m_strErr.c_str();
		}
};

#endif
