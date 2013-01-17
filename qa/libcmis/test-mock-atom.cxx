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

#define SERVER_URL string( "http://mockup/binding" )
#define SERVER_REPOSITORY string( "mock" )
#define SERVER_USERNAME "tester"
#define SERVER_PASSWORD "somepass"

#define private public
#define protected public

#include <mockup-config.h>
#include "atom-session.hxx"

using namespace std;

class AtomTest : public CppUnit::TestFixture
{
    public:
        void getRepositoriesTest( );
        void getRepositoriesBadAuthTest( );
        void sessionCreationTest( );
        void sessionCreationBadAuthTest( );
        void getTypeTest( );
        void getObjectTest( );

        CPPUNIT_TEST_SUITE( AtomTest );
        CPPUNIT_TEST( getRepositoriesTest );
        CPPUNIT_TEST( getRepositoriesBadAuthTest );
        CPPUNIT_TEST( sessionCreationTest );
        CPPUNIT_TEST( sessionCreationBadAuthTest );
        CPPUNIT_TEST( getTypeTest );
        CPPUNIT_TEST( getObjectTest );
        CPPUNIT_TEST_SUITE_END( );

        AtomPubSession getTestSession( string username = string( ), string password = string( ) );
        void loadFromFile( const char* path, string& buf );
};

CPPUNIT_TEST_SUITE_REGISTRATION( AtomTest );

void AtomTest::getRepositoriesTest( )
{
    // Response showing one mock repository
    curl_mockup_reset( );
    curl_mockup_setResponse( "data/atom-workspaces.xml" );

    list< libcmis::RepositoryPtr > actual = AtomPubSession::getRepositories( SERVER_URL, SERVER_USERNAME, SERVER_PASSWORD );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of repositories", size_t( 1 ), actual.size( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong repository found", SERVER_REPOSITORY, actual.front()->getId( ) );
}

void AtomTest::getRepositoriesBadAuthTest( )
{
    // Response showing one mock repository
    curl_mockup_reset( );
    curl_mockup_setResponse( "data/atom-workspaces.xml" );
    curl_mockup_setCredentials( SERVER_USERNAME, SERVER_PASSWORD );

    try
    {
        AtomPubSession::getRepositories( SERVER_URL, "baduser", "badpass" );
        CPPUNIT_FAIL( "Exception should have been thrown" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong error type", string( "permissionDenied" ), e.getType( ) );
    }
}

void AtomTest::sessionCreationTest( )
{
    // Response showing one mock repository
    curl_mockup_reset( );
    curl_mockup_setResponse( "data/atom-workspaces.xml" );
    curl_mockup_setCredentials( SERVER_USERNAME, SERVER_PASSWORD );

    AtomPubSession session( SERVER_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );

    // Check for the mandatory collection URLs
    CPPUNIT_ASSERT_MESSAGE( "root collection URL missing",
            !session.getAtomRepository()->getCollectionUrl( Collection::Root ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "types collection URL missing",
            !session.getAtomRepository()->getCollectionUrl( Collection::Types ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "query collection URL missing",
            !session.getAtomRepository()->getCollectionUrl( Collection::Query ).empty() );

    // The optional collection URLs are present on InMemory, so check them
    CPPUNIT_ASSERT_MESSAGE( "checkedout collection URL missing",
            !session.getAtomRepository()->getCollectionUrl( Collection::CheckedOut ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "unfiled collection URL missing",
            !session.getAtomRepository()->getCollectionUrl( Collection::Unfiled ).empty() );

    // Check for the mandatory URI template URLs
    CPPUNIT_ASSERT_MESSAGE( "objectbyid URI template URL missing",
            !session.getAtomRepository()->getUriTemplate( UriTemplate::ObjectById ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "objectbypath URI template URL missing",
            !session.getAtomRepository()->getUriTemplate( UriTemplate::ObjectByPath ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "typebyid URI template URL missing",
            !session.getAtomRepository()->getUriTemplate( UriTemplate::TypeById ).empty() );
    
    // The optional URI template URL is present on InMemory, so check it
    CPPUNIT_ASSERT_MESSAGE( "query URI template URL missing",
            !session.getAtomRepository()->getUriTemplate( UriTemplate::Query ).empty() );

    // Check that the root id is defined
    CPPUNIT_ASSERT_MESSAGE( "Root node ID is missing",
            !session.getRootId().empty() );
}

void AtomTest::sessionCreationBadAuthTest( )
{
    // Response showing one mock repository
    curl_mockup_reset( );
    curl_mockup_setResponse( "data/atom-workspaces.xml" );
    curl_mockup_setCredentials( SERVER_USERNAME, SERVER_PASSWORD );

    try
    {
        AtomPubSession session( SERVER_URL, SERVER_REPOSITORY, "bad", "bad" );
        CPPUNIT_FAIL( "Exception should have been thrown" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong error type", string( "permissionDenied" ), e.getType( ) );
    }
}

void AtomTest::getTypeTest( )
{
    curl_mockup_reset( );
    curl_mockup_addResponse( "http://mockup/mock/type", "id=cmis:folder", "data/atom-type-folder.xml" );
    curl_mockup_setCredentials( SERVER_USERNAME, SERVER_PASSWORD );

    AtomPubSession session = getTestSession( SERVER_USERNAME, SERVER_PASSWORD );

    string expectedId( "cmis:folder" );
    libcmis::ObjectTypePtr actual = session.getType( expectedId );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong Id for fetched type", expectedId, actual->getId( ) );
}

void AtomTest::getObjectTest( )
{
    curl_mockup_reset( );
    curl_mockup_addResponse( "http://mockup/mock/id", "id=valid-object", "data/atom-valid-object.xml" );
    curl_mockup_addResponse( "http://mockup/mock/type", "id=cmis:folder", "data/atom-type-folder.xml" );
    curl_mockup_setCredentials( SERVER_USERNAME, SERVER_PASSWORD );

    AtomPubSession session = getTestSession( SERVER_USERNAME, SERVER_PASSWORD );

    string expectedId( "valid-object" );
    libcmis::ObjectPtr actual = session.getObject( expectedId );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong Id for fetched object", expectedId, actual->getId( ) );
}

AtomPubSession AtomTest::getTestSession( string username, string password )
{
    AtomPubSession session;
    string buf;
    loadFromFile( "data/atom-workspaces.xml", buf );
    session.parseServiceDocument( buf );
    
    session.m_username = username;
    session.m_password = password;

    return session;
}

void AtomTest::loadFromFile( const char* path, string& buf )
{
    ifstream in( path );

    in.seekg( 0, ios::end );
    int length = in.tellg( );
    in.seekg( 0, ios::beg );

    char* buffer = new char[length];
    in.read( buffer, length );
    in.close( );

    buf = string( buffer, length );
    delete[] buffer;
}
