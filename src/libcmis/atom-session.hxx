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
#ifndef _ATOM_SESSION_HXX_
#define _ATOM_SESSION_HXX_

#include <list>
#include <map>
#include <string>

#include <libxml/xmlstring.h>
#include <libxml/xpath.h>

#include "exception.hxx"
#include "session.hxx"

struct Collection {
    enum Type
    {
        Root,
        Types,
        Query,
        CheckedOut,
        Unfiled
    };
};

struct UriTemplate {
    enum Type
    {
        ObjectById,
        ObjectByPath,
        TypeById,
        Query
    };

    static std::string createUrl( const std::string& pattern, std::map< std::string, std::string > variables );
};

class AtomPubSession : public libcmis::Session
{
    private:
        std::string m_sAtomPubUrl;
        std::string m_sRepository;
        std::string m_username;
        std::string m_password;
        std::string m_sRootId;

        // Collections URLs
        std::map< Collection::Type, std::string > m_aCollections;

        // URI templates
        std::map< UriTemplate::Type, std::string > m_aUriTemplates;

    public:
        AtomPubSession( std::string sAtomPubUrl, std::string repository,
                        std::string username, std::string password ) throw ( libcmis::Exception );
        ~AtomPubSession( );

        static std::list< std::string > getRepositories( std::string url,
                        std::string username, std::string password ) throw ( libcmis::Exception );

        std::string getCollectionUrl( Collection::Type );

        std::string getUriTemplate( UriTemplate::Type );

        std::string getRootId( ) { return m_sRootId; }

        std::string getUsername( ) { return m_username; }

        std::string getPassword( ) { return m_password; }

        // Utility methods

        libcmis::FolderPtr getFolder( std::string id );

        libcmis::ObjectPtr createObjectFromEntryDoc( xmlDocPtr doc );

        // Override session methods

        virtual libcmis::FolderPtr getRootFolder();

        virtual libcmis::ObjectPtr getObject( std::string id ) throw ( libcmis::Exception );

    private:
        void readCollections( xmlNodeSetPtr pNodeSet );
        void readUriTemplates( xmlNodeSetPtr pNodeSet );
};

#endif
