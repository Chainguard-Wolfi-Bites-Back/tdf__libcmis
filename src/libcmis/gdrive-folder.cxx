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
 * Copyright (C) 2013 Cao Cuong Ngo <cao.cuong.ngo@gmail.com>
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

#include "gdrive-folder.hxx"
#include "gdrive-session.hxx"

using namespace std;
using libcmis::PropertyPtrMap;

GDriveFolder::GDriveFolder( GDriveSession* session ):
    libcmis::Object( session ),
    libcmis::Folder( session ),
    GDriveObject( session )
{
}

GDriveFolder::GDriveFolder( GDriveSession* session, Json json ):
    libcmis::Object( session ),
    libcmis::Folder( session ),
    GDriveObject( session, json )
{
}

GDriveFolder::~GDriveFolder( )
{
}

vector< libcmis::ObjectPtr > GDriveFolder::getChildren( ) 
    throw ( libcmis::Exception )
{
    vector< libcmis::ObjectPtr > children;
    
    string childrenUrl = getSession( )->getBaseUrl() + "/files/" + getId( ) + 
                                                                "/children";    
    string res;
    try
    {
        res = getSession( )->httpGetRequest( childrenUrl )->getStream()->str();
    }
    catch ( const CurlException& e )
    {
        throw e.getCmisException( );
    }

    Json jsonRes = Json::parse( res );
    Json::JsonVector objs = jsonRes["items"].getList( );
    // Create children objects from Json objects
    for(unsigned int i = 0; i < objs.size(); i++)
    {   
        libcmis::ObjectPtr child( new GDriveObject( getSession(), objs[i] ) );
        children.push_back( child );
    }   
    
    return children;
}

libcmis::FolderPtr GDriveFolder::createFolder( 
    const PropertyPtrMap& /*properties*/ ) 
        throw( libcmis::Exception )
{
    libcmis::FolderPtr folderPtr;
    string mimeType = "application/vnd.google-apps.folder";

    // TODO implementation   

    return folderPtr;
}
    
libcmis::DocumentPtr GDriveFolder::createDocument( 
    const PropertyPtrMap& /*properties*/, 
    boost::shared_ptr< ostream > /*os*/, 
    string /*contentType*/, string ) 
    throw ( libcmis::Exception )
{
    //TODO implementation
    libcmis::DocumentPtr result;
    return result;
}

vector< string > GDriveFolder::removeTree( 
    bool /*allVersions*/, 
    libcmis::UnfileObjects::Type /*unfile*/, 
    bool /*continueOnError*/ ) 
        throw ( libcmis::Exception )
{
    //TODO implementation
    vector< string > result;
    return result;
}

