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
 * Copyright (C) 2014 Mihai Varga <mihai.mv13@gmail.com>
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
#include "oauth2-handler.hxx"
#include "onedrive-session.hxx"
#include "onedrive-repository.hxx"
#include "json-utils.hxx"

using namespace std;

OneDriveSession::OneDriveSession ( string baseUrl,
                               string username,
                               string password,
                               libcmis::OAuth2DataPtr oauth2,
                               bool verbose )
                                    throw ( libcmis::Exception ) :
    BaseSession( baseUrl, string(), username, password, false,
                 libcmis::OAuth2DataPtr(), verbose )

{
    // Add the dummy repository
    m_repositories.push_back( getRepository( ) );

    if ( oauth2 && oauth2->isComplete( ) ){
        setOAuth2Data( oauth2 );
    }
}

OneDriveSession::OneDriveSession( const OneDriveSession& copy ) :
    BaseSession( copy )
{
}

OneDriveSession::OneDriveSession() :
    BaseSession()
{
}

OneDriveSession::~OneDriveSession()
{
}

libcmis::RepositoryPtr OneDriveSession::getRepository( )
    throw ( libcmis::Exception )
{
    // Return a dummy repository since OneDrive doesn't have that notion
    // I think a Gdrive repo will do for the moment
    libcmis::RepositoryPtr repo( new OneDriveRepository( ) );
    return repo;
}

libcmis::ObjectPtr OneDriveSession::getObject( string objectId )
    throw ( libcmis::Exception )
{
    // Run the http request to get the properties definition
    string res;
    string objectLink = m_bindingUrl + "/" + objectId;
    try
    {
        res = httpGetRequest( objectLink )->getStream()->str();
    }
    catch ( const CurlException& e )
    {
        throw e.getCmisException( );
    }
    Json jsonRes = Json::parse( res );

    // If we have a folder, then convert the object
    libcmis::ObjectPtr object;
    string kind = jsonRes["type"].toString( );
    if ( kind == "file" )
    {
    }
    return object;
}

libcmis::ObjectPtr OneDriveSession::getObjectByPath( string path )
    throw ( libcmis::Exception )
{
    return getObject( path );
}

libcmis::ObjectTypePtr OneDriveSession::getType( string id )
    throw ( libcmis::Exception )
{
    id += "";
    libcmis::ObjectTypePtr type;
    return type;
}

vector< libcmis::ObjectTypePtr > OneDriveSession::getBaseTypes( )
    throw ( libcmis::Exception )
{
    vector< libcmis::ObjectTypePtr > types;
    return types;
}
