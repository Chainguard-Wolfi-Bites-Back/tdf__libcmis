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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "ws-session.hxx"
#include "xml-utils.hxx"

using namespace std;

WSSession::WSSession( string bindingUrl, string repositoryId, 
        string username, string password, bool verbose ) throw ( libcmis::Exception ) :
    BaseSession( bindingUrl, repositoryId, username, password, verbose ),
    m_servicesUrls( )
{
    initialize( );
}

WSSession::WSSession( const WSSession& copy ) :
    BaseSession( copy ),
    m_servicesUrls( copy.m_servicesUrls )
{
}


WSSession& WSSession::operator=( const WSSession& copy )
{
    BaseSession::operator=( copy );
    m_servicesUrls = copy.m_servicesUrls;
    
    return *this;
}

WSSession::~WSSession( )
{
}

string WSSession::getWsdl( string url ) throw ( CurlException )
{
    string buf = httpGetRequest( url )->str( );

    // Do we have a wsdl file?
    bool isWsdl = false;

    xmlDocPtr doc = xmlReadMemory( buf.c_str(), buf.size(), m_bindingUrl.c_str(), NULL, 0 );
    if ( NULL != doc )
    {
        xmlXPathContextPtr xpathCtx = xmlXPathNewContext( doc );
        libcmis::registerCmisWSNamespaces( xpathCtx );

        if ( NULL != xpathCtx )
        {
            string definitionsXPath( "/wsdl:definitions" );
            xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression( BAD_CAST( definitionsXPath.c_str() ), xpathCtx );

            isWsdl = ( xpathObj != NULL ) && ( xpathObj->nodesetval != NULL ) && ( xpathObj->nodesetval->nodeNr > 0 );
        }
        xmlXPathFreeContext( xpathCtx );
    }

    // If we don't have a wsdl file we may have received an HTML explanation for it,
    // try to add ?wsdl to the URL (last chance to get something)
    if ( !isWsdl )
    {
        if ( url.find( "?" ) == string::npos )
            url += "?";
        else
            url += "&";
        url += "wsdl";

        buf = httpGetRequest( url )->str( );
    }

    return buf;
}

void WSSession::initialize( ) throw ( libcmis::Exception )
{
    if ( m_repositoriesIds.empty() )
    {
        // Get the wsdl file
        string buf;
        try
        {
            buf = getWsdl( m_bindingUrl );
        }
        catch ( const CurlException& e )
        {
            throw e.getCmisException( );
        }
       
        // parse the content
        xmlDocPtr doc = xmlReadMemory( buf.c_str(), buf.size(), m_bindingUrl.c_str(), NULL, 0 );

        if ( NULL != doc )
        {
            // Get all the services soap URLs
            m_servicesUrls.clear( );

            xmlXPathContextPtr xpathCtx = xmlXPathNewContext( doc );
            libcmis::registerCmisWSNamespaces( xpathCtx );

            if ( NULL != xpathCtx )
            {
                string serviceXPath( "//wsdl:service" );
                xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression( BAD_CAST( serviceXPath.c_str() ), xpathCtx );

                if ( xpathObj != NULL )
                {
                    int nbServices = 0;
                    if ( xpathObj->nodesetval )
                        nbServices = xpathObj->nodesetval->nodeNr;

                    for ( int i = 0; i < nbServices; i++ )
                    {
                        // What service do we have here?
                        xmlNodePtr node = xpathObj->nodesetval->nodeTab[i];
                        string name = libcmis::getXmlNodeAttributeValue( node, "name" );

                        // Gimme you soap:address location attribute
                        string locationXPath = serviceXPath + "[@name='" + name + "']/wsdl:port/soap:address/attribute::location";
                        string location = libcmis::getXPathValue( xpathCtx, locationXPath );

                        m_servicesUrls[name] = location;
                    }
                }
            }
            xmlXPathFreeContext( xpathCtx );
        }
        else
            throw libcmis::Exception( "Failed to parse service document" );

        xmlFreeDoc( doc );

        // TODO Get all repositories Ids
    }
}

string WSSession::getServiceUrl( string name )
{
    string url;

    map< string, string >::iterator it = m_servicesUrls.find( name );
    if ( it != m_servicesUrls.end( ) )
        url = it->second;

    return url;
}

list< string > WSSession::getRepositories( string url, string username, string password, bool verbose ) throw ( libcmis::Exception )
{
    WSSession session( url, string(), username, password, verbose );
    session.initialize( );
    return session.m_repositoriesIds;
}

libcmis::RepositoryPtr WSSession::getRepository( ) throw ( libcmis::Exception )
{
    libcmis::RepositoryPtr empty;

    // TODO Implement me
    return empty;
}

libcmis::ObjectPtr WSSession::getObject( string id ) throw ( libcmis::Exception )
{
    libcmis::ObjectPtr empty;

    // TODO Implement me
    return empty;
}

libcmis::ObjectPtr WSSession::getObjectByPath( string path ) throw ( libcmis::Exception )
{
    libcmis::ObjectPtr empty;

    // TODO Implement me
    return empty;
}

libcmis::ObjectTypePtr WSSession::getType( string id ) throw ( libcmis::Exception )
{
    libcmis::ObjectTypePtr empty;

    // TODO Implement me
    return empty;
}

