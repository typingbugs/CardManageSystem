//#pragma once
#ifndef _H_LOG_H_
#define _H_LOG_H_

#include <stdio.h>
typedef unsigned char uchar_t;

class CHLog
{
public:
	CHLog(void)
	{
		m_fp = fopen( "hfsim.log", "w");
	}
	~CHLog(void)
	{
		if( m_fp )
		{
			fclose( m_fp );
		}
	}

public:
	void logstr( char *str )
	{
		if( m_fp )
		{
			fprintf( m_fp, "%s\n", str );
		}
	}

	void logDIM( int len, uchar_t aucDat[] )
	{
		int n = len/16;
		int k = 0;

		if( m_fp )
		{
			fprintf( m_fp, "hex----------------\n");
			for( int i = 0; i < n; i++ )
			{
				for( int j = 0; j < 16; j++ )
				{
					fprintf( m_fp, "%02x ", aucDat[k++]);
				}
				fprintf( m_fp, "\n" );
			}

			for( int j = 0; j < len%16; j++ )
			{
				fprintf( m_fp, "%02x ", aucDat[k++]);
			}
			fprintf( m_fp, "\n" );
			fprintf( m_fp, "hex----------------\n");
		}
	}

protected:
	FILE *m_fp;
};

#endif

