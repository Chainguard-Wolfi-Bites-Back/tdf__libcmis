/* libcmis
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 SUSE <cbosdonnat@suse.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */
#ifndef _ATOM_UTILS_HXX_
#define _ATOM_UTILS_HXX_

#include <ostream>
#include <string>

#include <boost/date_time.hpp>
#include <curl/curl.h>
#include <libxml/xpathInternals.h>

#include "exception.hxx"

#define NS_APP_URL BAD_CAST( "http://www.w3.org/2007/app" )
#define NS_ATOM_URL BAD_CAST( "http://www.w3.org/2005/Atom" )
#define NS_CMIS_URL BAD_CAST( "http://docs.oasis-open.org/ns/cmis/core/200908/" )
#define NS_CMISRA_URL BAD_CAST( "http://docs.oasis-open.org/ns/cmis/restatom/200908/" )

#define URI_TEMPLATE_VAR_ID std::string( "id" )

namespace atom
{
    class CurlException : public std::exception
    {
        private:
            std::string m_message;
            CURLcode    m_code;

        public:
            CurlException( std::string message, CURLcode code ) :
                exception( ),
                m_message( message ),
                m_code( code )
            {
            }

            ~CurlException( ) throw () { }
            virtual const char* what( ) const throw ();

            CURLcode getErrorCode( ) const { return m_code; }
            std::string getErrorMessage( ) const { return m_message; }

            libcmis::Exception getCmisException ( ) const;
    };

    /** Class used to decode a stream.

        An instance of this class can hold remaining un-decoded data to use
        for a future decode call.
      */
    class EncodedData
    {
        private:
            FILE* m_stream;
            std::ostream* m_outStream;

            std::string m_encoding;
            unsigned long m_pendingValue;
            int m_pendingRank;
            size_t m_missingBytes;

        public:
            EncodedData( FILE* stream );
            EncodedData( std::ostream* stream );
            EncodedData( const EncodedData& rCopy );

            const EncodedData& operator=( const EncodedData& rCopy );

            void setEncoding( std::string encoding ) { m_encoding = encoding; }
            void decode( void* buf, size_t size, size_t nmemb );
            void finish( );

        private:
            void write( void* buf, size_t size, size_t nmemb );
            void decodeBase64( const char* buf, size_t len );
    };
    
    void registerNamespaces( xmlXPathContextPtr pXPathCtx );

    std::string getXPathValue( xmlXPathContextPtr pXPathCtx, std::string req ); 

    xmlDocPtr wrapInDoc( xmlNodePtr entryNode );

    std::string httpGetRequest( std::string url, const std::string& username,
                                const std::string& password, bool verbose ) throw ( CurlException );
   
    /** Parse a xsd:dateTime string and return the corresponding UTC posix time.
     */ 
    boost::posix_time::ptime parseDateTime( std::string dateTimeStr );
}

#endif
