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
#ifndef _ATOM_OBJECT_HXX_
#define _ATOM_OBJECT_HXX_

#include <libxml/tree.h>

#include "atom-allowable-actions.hxx"
#include "object.hxx"

class AtomPubSession;

class AtomLink
{
    private:
        std::string m_rel;
        std::string m_type;
        std::string m_id;
        std::string m_href;

    public:
        AtomLink( xmlNodePtr node ) throw ( libcmis::Exception );

        std::string getRel( ) { return m_rel; }
        std::string getType( ) { return m_type; }
        std::string getId( ) { return m_id; }
        bool hasId( ) { return !m_id.empty( ); }
        std::string getHref( ) { return m_href; }
};

class AtomObject : public virtual libcmis::Object
{
    private:
        AtomPubSession* m_session;
        time_t m_refreshTimestamp;

        std::string m_infosUrl;
        std::string m_typeId;
        libcmis::ObjectTypePtr m_typeDescription;

        std::map< std::string, libcmis::PropertyPtr > m_properties;
        boost::shared_ptr< AtomAllowableActions > m_allowableActions;

        std::vector< AtomLink > m_links;

    public:
        AtomObject( AtomPubSession* session ) throw ( libcmis::Exception );
        AtomObject( const AtomObject& copy );
        ~AtomObject( );

        AtomObject& operator=( const AtomObject& copy );

        // Overridden methods from libcmis::Object
        virtual std::string getId( ); 
        virtual std::string getName( );
        virtual std::vector< std::string > getPaths( );
        
        virtual std::string getBaseType( );
        virtual std::string getType( );
            
        virtual std::string getCreatedBy( );
        virtual boost::posix_time::ptime getCreationDate( );
        virtual std::string getLastModifiedBy( );
        virtual boost::posix_time::ptime getLastModificationDate( );

        virtual std::string getChangeToken( );
        virtual bool isImmutable( );

        virtual std::map< std::string, libcmis::PropertyPtr >& getProperties( );
        virtual void updateProperties( ) throw ( libcmis::Exception );

        virtual libcmis::ObjectTypePtr getTypeDescription( );
        virtual boost::shared_ptr< libcmis::AllowableActions > getAllowableActions( );

        /** Reload the data from the server.
              */
        virtual void refresh( ) throw ( libcmis::Exception ) { refreshImpl( NULL ); }
        virtual time_t getRefreshTimestamp( ) { return m_refreshTimestamp; }

        virtual void remove( bool allVersion = true ) throw ( libcmis::Exception );

        virtual std::string toString( );

        virtual void toXml( xmlTextWriterPtr writer );

    protected:

        std::string& getInfosUrl( ) { return m_infosUrl; }
        virtual void refreshImpl( xmlDocPtr doc ) throw ( libcmis::Exception );
        virtual void extractInfos( xmlDocPtr doc );

        AtomPubSession* getSession( ) { return m_session; }

        /** Documents will override this method to output the content stream
          */
        virtual void contentToXml( xmlTextWriterPtr writer );

        /** Get the atom link corresponding to the given relation and type or NULL
            if no link matched those criteria.

            \param rel the relation to match
            \param type the type to match or the empty string to match all types.
          */
        AtomLink* getLink( std::string rel, std::string type );
};

#endif
