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
#include "atom-content.hxx"
#include "atom-folder.hxx"
#include "atom-utils.hxx"

using namespace std;

namespace
{
}

AtomFolder::AtomFolder( string url ) :
    AtomResource( url ),
    m_path( ),
    m_childrenUrl( )
{
    string buf  = atom::httpGetRequest( getInfosUrl() );
    
    xmlDocPtr doc = xmlReadMemory( buf.c_str(), buf.size(), getInfosUrl().c_str(), NULL, 0 );
    if ( NULL != doc )
        extractInfos( doc );
    else
    {
        fprintf( stderr, "Failed to parse folder infos\n" );
    }
    xmlFreeDoc( doc );
}

AtomFolder::AtomFolder( xmlNodePtr entryNd ) :
    AtomResource( string() ),
    m_path( ),
    m_childrenUrl( )
{
    xmlDocPtr doc = atom::wrapInDoc( entryNd );
    extractInfos( doc );
    xmlFreeDoc( doc );
}


AtomFolder::~AtomFolder( )
{
}

vector< ResourcePtr > AtomFolder::getChildren( )
{
    vector< ResourcePtr > children;
    
    string buf = atom::httpGetRequest( m_childrenUrl );

    xmlDocPtr doc = xmlReadMemory( buf.c_str(), buf.size(), m_childrenUrl.c_str(), NULL, 0 );
    if ( NULL != doc )
    {
        xmlXPathContextPtr pXPathCtx = xmlXPathNewContext( doc );
        atom::registerNamespaces( pXPathCtx );
        if ( NULL != pXPathCtx )
        {
            const string& entriesReq( "//atom:entry" );
            xmlXPathObjectPtr pXPathObj = xmlXPathEvalExpression( BAD_CAST( entriesReq.c_str() ), pXPathCtx );

            if ( NULL != pXPathObj && NULL != pXPathObj->nodesetval )
            {
                int size = pXPathObj->nodesetval->nodeNr;
                for ( int i = 0; i < size; i++ )
                {
                    xmlNodePtr node = pXPathObj->nodesetval->nodeTab[i];

                    ResourcePtr resource;

                    xmlDocPtr entryDoc = atom::wrapInDoc( node );
                    if ( !AtomFolder::getChildrenUrl( entryDoc ).empty() )
                    {
                        ResourcePtr folder( new AtomFolder( node ) );
                        resource.swap( folder );
                    }
                    else
                    {
                        ResourcePtr content( new AtomContent( node ) );
                        resource.swap( content );
                    }

                    if ( resource.get() )
                        children.push_back( resource );
                    xmlFreeDoc( entryDoc );
                }
            }

            xmlXPathFreeObject( pXPathObj );
        }

        xmlXPathFreeContext( pXPathCtx );
    }
    else
    {
        fprintf( stderr, "Failed to parse folder infos\n" );
    }
    xmlFreeDoc( doc );

    return children;
}

string AtomFolder::getName( )
{
    return AtomResource::getName( );
}

string AtomFolder::getPath( )
{
    return m_path;
}

void AtomFolder::extractInfos( xmlDocPtr doc )
{
    AtomResource::extractInfos( doc );
    m_childrenUrl = AtomFolder::getChildrenUrl( doc );

    xmlXPathContextPtr pXPathCtx = xmlXPathNewContext( doc );

    // Register the Service Document namespaces
    atom::registerNamespaces( pXPathCtx );

    if ( NULL != pXPathCtx )
    {
        // Get the path
        string pathReq( "//cmis:propertyString[@queryName='cmis:path']/cmis:value/text()" );
        m_path = atom::getXPathValue( pXPathCtx, pathReq );
    }
    xmlXPathFreeContext( pXPathCtx );
}

string AtomFolder::getChildrenUrl( xmlDocPtr doc )
{
    string childrenUrl;

    xmlXPathContextPtr pXPathCtx = xmlXPathNewContext( doc );
    atom::registerNamespaces( pXPathCtx );

    if ( NULL != pXPathCtx )
    {
        // Get the children collection url
        string downReq( "//atom:link[@rel='down' and @type='application/atom+xml;type=feed']//attribute::href" );
        childrenUrl = atom::getXPathValue( pXPathCtx, downReq );
    }
    xmlXPathFreeContext( pXPathCtx );

    return childrenUrl;
}
