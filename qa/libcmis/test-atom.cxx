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

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>

#include "atom-document.hxx"
#include "atom-folder.hxx"
#include "atom-session.hxx"
#include "test-helpers.hxx"

// InMemory local test server data
#define SERVER_ATOM_URL string( "http://localhost:8080/inmemory/atom" )
#define SERVER_REPOSITORY string( "A1" )
#define SERVER_USERNAME string( "tester" )
#define SERVER_PASSWORD string( "somepass" )

using boost::shared_ptr;
using namespace std;

class AtomTest : public CppUnit::TestFixture
{
    public:

        // Node operations tests

        void checkOutTest( );
        void cancelCheckOutTest( );
        void checkInTest( );
        void getAllVersionsTest( );
        void moveTest( );

        CPPUNIT_TEST_SUITE( AtomTest );
        CPPUNIT_TEST( checkOutTest );
        CPPUNIT_TEST( cancelCheckOutTest );
        CPPUNIT_TEST( checkInTest );
        CPPUNIT_TEST( getAllVersionsTest );
        CPPUNIT_TEST( moveTest );
        CPPUNIT_TEST_SUITE_END( );
};

void AtomTest::checkOutTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );

    // First create a document of type VersionableType
    libcmis::DocumentPtr doc = test::createVersionableDocument( &session, "checkOutTest" );
    CPPUNIT_ASSERT_MESSAGE( "Failed to create versionable document", doc.get() != NULL );

    libcmis::DocumentPtr pwc = doc->checkOut( );
    
    CPPUNIT_ASSERT_MESSAGE( "Missing returned Private Working Copy", pwc.get( ) != NULL );

    map< string, libcmis::PropertyPtr >::iterator it = pwc->getProperties( ).find( string( "cmis:isVersionSeriesCheckedOut" ) );
    CPPUNIT_ASSERT_MESSAGE( "cmis:isVersionSeriesCheckedOut property is missing", it != pwc->getProperties( ).end( ) );
    vector< bool > values = it->second->getBools( );
    CPPUNIT_ASSERT_MESSAGE( "cmis:isVersionSeriesCheckedOut isn't true", values.front( ) );
}

void AtomTest::cancelCheckOutTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );

    // First create a versionable document and check it out
    libcmis::DocumentPtr doc = test::createVersionableDocument( &session, "cancelCheckOutTest" );
    libcmis::DocumentPtr pwc = doc->checkOut( );

    CPPUNIT_ASSERT_MESSAGE( "Failed to create Private Working Copy document", pwc.get() != NULL );

    string id = pwc->getId( );
    pwc->cancelCheckout( );

    // Check that the PWC doesn't exist any more
    try
    {
        session.getObject( id );
        CPPUNIT_FAIL( "Private Working Copy object should no longer exist" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception message", string( "No such node: " ) + id, string( e.what() ) );
    }
}

void AtomTest::checkInTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );

    // First create a versionable document and check it out
    libcmis::DocumentPtr doc = test::createVersionableDocument( &session, "checkInTest" );
    libcmis::DocumentPtr pwc = doc->checkOut( );

    CPPUNIT_ASSERT_MESSAGE( "Failed to create Private Working Copy document", pwc.get() != NULL );

    // Do the checkin
    bool isMajor = true;
    string comment( "Some check-in comment" );
    map< string, libcmis::PropertyPtr > properties;
    string newContent = "Some New content to check in";
    boost::shared_ptr< ostream > stream ( new stringstream( newContent ) );
    pwc->checkIn( isMajor, comment, properties, stream, "text/plain", "filename.txt" );

    map< string, libcmis::PropertyPtr > actualProperties = pwc->getProperties( );

    {
        map< string, libcmis::PropertyPtr >::iterator it = actualProperties.find( "cmis:isLatestVersion" );
        CPPUNIT_ASSERT_MESSAGE( "cmis:isLatestVersion isn't true", it->second->getBools().front( ) );
    }
    
    {
        map< string, libcmis::PropertyPtr >::iterator it = actualProperties.find( "cmis:isMajorVersion" );
        CPPUNIT_ASSERT_MESSAGE( "cmis:isMajorVersion isn't true", it->second->getBools().front( ) );
    }

    {
        map< string, libcmis::PropertyPtr >::iterator it = actualProperties.find( "cmis:checkinComment" );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "cmis:checkinComment doesn't match", comment, it->second->getStrings().front( ) );
    }
}

void AtomTest::getAllVersionsTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );

    // First create a versionable document and check it out
    libcmis::DocumentPtr doc = test::createVersionableDocument( &session, "getAllVersionsTest" );
    libcmis::DocumentPtr pwc = doc->checkOut( );

    CPPUNIT_ASSERT_MESSAGE( "Failed to create Private Working Copy document", pwc.get() != NULL );

    // Create a version
    bool isMajor = true;
    string comment( "Some check-in comment" );
    map< string, libcmis::PropertyPtr > properties;
    string newContent = "Some New content to check in";
    boost::shared_ptr< ostream > stream ( new stringstream( newContent ) );
    libcmis::DocumentPtr newVersion = pwc->checkIn( isMajor, comment, properties, stream, "text/plain", "filename.txt" );

    // Get all the versions (method to check)
    vector< libcmis::DocumentPtr > versions = newVersion->getAllVersions( ); 

    // Checks
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of versions", size_t( 2 ), versions.size( ) );
}

void AtomTest::moveTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );
    
    string id( "135" );
    libcmis::ObjectPtr object = session.getObject( id );
    libcmis::Document* document = dynamic_cast< libcmis::Document* >( object.get() );
    CPPUNIT_ASSERT_MESSAGE( "Document to move is missing", document != NULL );

    libcmis::FolderPtr src = document->getParents( ).front( );
    libcmis::FolderPtr dest = session.getFolder( "101" );

    document->move( src, dest );

    vector< libcmis::FolderPtr > parents = document->getParents( );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong parents size", size_t( 1 ), parents.size( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong parent", string( "101" ), parents.front( )->getId( ) );
}

CPPUNIT_TEST_SUITE_REGISTRATION( AtomTest );
