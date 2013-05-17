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

#include <libxml/HTMLparser.h>
#include <libxml/xmlreader.h>

#include "gdrive-utils.hxx"
#include "json-utils.hxx"
#include "xml-utils.hxx"

using std::string;
using libcmis::PropertyPtrMap;

string GdriveUtils::toCmisKey( const string& key )
{
    string convertedKey;
    if ( key == "id")
        convertedKey = "cmis:objectId";
    else if ( key == "ownerNames" )
        convertedKey = "cmis:createdBy";
    else if ( key == "description" )
        convertedKey = "cmis:description";
    else if ( key == "createdDate" )
        convertedKey = "cmis:creationDate";
    else if ( key == "lastModifyingUserName" )
        convertedKey = "cmis:lastModifiedBy";
    else if ( key == "modifiedDate" )
        convertedKey = "cmis:lastModificationDate";
    else if ( key == "title" )
        convertedKey = "cmis:contentStreamFileName";
    else if ( key == "mimeType" )
        convertedKey = "cmis:contentStreamMimeType";
    else if ( key == "fileSize" )
        convertedKey = "cmis:contentStreamLength";
    else if ( key == "editable" )
        convertedKey = "cmis:isImmutable";
    else if ( key == "parents" )
        convertedKey = "cmis:parentId";
    else convertedKey = key;
    return convertedKey;
}

string GdriveUtils::toGdriveKey( const string& key )
{
    string convertedKey;
    if ( key == "cmis:objectId")
        convertedKey = "id";
    else if ( key == "cmis:createdBy" )
        convertedKey = "ownerNames";
    else if ( key == "cmis:creationDate" )
        convertedKey = "createdDate";
    else if ( key == "cmis:description" )
        convertedKey = "description";
    else if ( key == "cmis:lastModifiedBy" )
        convertedKey = "lastModifyingUserName";
    else if ( key == "cmis:lastModificationDate" )
        convertedKey = "modifiedDate";
    else if ( key == "cmis:contentStreamFileName" )
        convertedKey = "title";
    else if ( key == "cmis:name" )
        convertedKey = "title";
    else if ( key == "cmis:contentStreamMimeType" )
        convertedKey = "mimeType";
    else if ( key == "cmis:contentStreamLength" )
        convertedKey = "fileSize";
    else if ( key == "cmis:isImmutable" )
        convertedKey = "editable";
    else if ( key == "cmis:parentId" )
        convertedKey = "parents";
    else convertedKey = key;
    return convertedKey;
}

Json GdriveUtils::toGdriveJson( const PropertyPtrMap& properties )
{
    Json propsJson;
    
    // check if cmis:name and cmis:contentStreamFileName has been duplicated
    bool duplicated = false;
    for ( PropertyPtrMap::const_iterator it = properties.begin() ; 
            it != properties.end() ; ++it )
    {
        string key = it->first;
        Json value( it->second );
        
        // Convert the key back to the gdrive key
        // take one of the two: cmis:name and cmis:contentStreamFileName
        if ( key != "cmis:name" || key != "cmis:contentStreamFleName" || !duplicated )
            propsJson.add( toGdriveKey( key ), value );

        if ( key == "cmis:name" || key =="cmis:contentStreamFileName" )
            duplicated = true;
    }

    return propsJson;
} 

bool GdriveUtils::checkUpdatable( const string& key )
{
    bool updatable = false;

    // taken from https://developers.google.com/drive/v2/reference/files
    updatable = ( key == "title" ||
                  key == "mimeType" ||
                  key == "description" ||
                  key == "modifiedDate" ||
                  key == "labels" ||
                  key == "parents" ||
                  key == "lastViewedByMeDate" );
    return updatable;    
}

int GdriveUtils::parseResponse ( const char* response, string& post, string& link )
{
    xmlDoc *doc = htmlReadDoc ( BAD_CAST( response ), NULL, 0,
            HTML_PARSE_NOWARNING | HTML_PARSE_RECOVER | HTML_PARSE_NOERROR );
    if ( doc == NULL ) return 0;
    xmlTextReaderPtr reader =   xmlReaderWalker( doc );
    if ( reader == NULL ) return 0;
    while ( true )
    {
        // Go to the next node, quit if not found
        if ( xmlTextReaderRead ( reader ) != 1) break;
        xmlChar* nodeName = xmlTextReaderName ( reader );
        if ( nodeName == NULL ) continue;
        // Find the redirect link
        if ( xmlStrEqual( nodeName, BAD_CAST( "form" ) ) )
        {
            xmlChar* action = xmlTextReaderGetAttribute( reader, 
                                                         BAD_CAST( "action" ));
            if ( action != NULL )
            {
                if ( xmlStrlen(action) > 0)
                    link = string ( (char*) action);
                xmlFree (action);
            }
        }
        // Find input values
        if ( !xmlStrcmp( nodeName, BAD_CAST( "input" ) ) )
        {
            xmlChar* name = xmlTextReaderGetAttribute( reader, 
                                                       BAD_CAST( "name" ));
            xmlChar* value = xmlTextReaderGetAttribute( reader, 
                                                        BAD_CAST( "value" ));
            if ( ( name != NULL ) && ( value!= NULL ) )
            {
                if ( ( xmlStrlen( name ) > 0) && ( xmlStrlen( value ) > 0) )
                {
                    post += libcmis::escape( ( char * ) name ); 
                    post += string ( "=" ); 
                    post += libcmis::escape( ( char * ) value ); 
                    post += string ( "&" );
                }
            }
            xmlFree( name );
            xmlFree( value );
        }
        xmlFree( nodeName );
    }
    xmlFreeTextReader( reader );              
    xmlFreeDoc( doc );
    if ( link.empty( ) || post.empty () ) 
        return 0;
    return 1;
}

string GdriveUtils::parseCode ( const char* response )
{
    string authCode;
    xmlDoc *doc = htmlReadDoc ( BAD_CAST( response ), NULL, 0,
            HTML_PARSE_NOWARNING | HTML_PARSE_RECOVER | HTML_PARSE_NOERROR );
    if ( doc == NULL ) return authCode;
    xmlTextReaderPtr reader = xmlReaderWalker( doc );
    if ( reader == NULL ) return authCode;

    while ( true )
    {
        // Go to the next node, quit if not found
        if ( xmlTextReaderRead ( reader ) != 1) break;
        xmlChar* nodeName = xmlTextReaderName ( reader );
        if ( nodeName == NULL ) continue;
        // Find the code 
        if ( xmlStrEqual( nodeName, BAD_CAST ( "input" ) ) )
        { 
            xmlChar* id = xmlTextReaderGetAttribute( reader, BAD_CAST( "id" ));
            if ( id != NULL )
            {
                if ( xmlStrEqual( id, BAD_CAST ( "code" ) ) )
                {
                    xmlChar* code = xmlTextReaderGetAttribute( 
                        reader, BAD_CAST("value") );
                    if ( code!= NULL )
                    {
                        authCode = string ( (char*) code );
                        xmlFree( code );
                    }
                }
                xmlFree ( id );
            }
        }
        xmlFree( nodeName );
    }
    xmlFreeTextReader( reader );              
    xmlFreeDoc( doc );

    return authCode;
}

Json GdriveUtils::createJsonFromParentId( const string& parentId )
{
    Json parentValue( parentId.c_str( ) );
    
    // parents is a Json array
    Json firstParent;
    firstParent.add( "id", parentValue );
    
    Json::JsonVector parents;
    parents.insert( parents.begin( ), firstParent );
    
    Json parentsValue( parents );

    return parentsValue;
}


