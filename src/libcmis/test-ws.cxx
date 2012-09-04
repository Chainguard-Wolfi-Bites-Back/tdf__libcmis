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

#include <document.hxx>
#include <ws-requests.hxx>
#include <ws-session.hxx>

#define SERVER_WSDL_URL string( "http://localhost:8080/inmemory/services/RepositoryService" )
#define SERVER_REPOSITORIES_COUNT list< string >::size_type( 1 )
#define SERVER_REPOSITORY string( "A1" )
#define SERVER_USERNAME string( "tester" )
#define SERVER_PASSWORD string( "somepass" )

using namespace std;

class WSTest : public CppUnit::TestFixture
{
    public:

        // Generic session factory tests

        void getRepositoriesTest( );
        void sessionCreationTest( );
        void getRepositoryTest( );
        void getRepositoryBadTest( );

        // Types tests

        void getTypeDefinitionTest( );
        void getTypeDefinitionErrorTest( );
        void getTypeChildrenTest( );

        // Object tests
        void getObjectTest( );
        void getObjectDocumentTest( );
        void getByPathValidTest( );
        void getByPathInvalidTest( );
        void updatePropertiesTest( );
        void deleteObjectTest( );
        void deleteTreeTest( );
        void getContentStreamTest( );

        CPPUNIT_TEST_SUITE( WSTest );
        CPPUNIT_TEST( getRepositoriesTest );
        CPPUNIT_TEST( sessionCreationTest );
        CPPUNIT_TEST( getRepositoryTest );
        CPPUNIT_TEST( getRepositoryBadTest );
        CPPUNIT_TEST( getTypeDefinitionTest );
        CPPUNIT_TEST( getTypeDefinitionErrorTest );
        CPPUNIT_TEST( getTypeChildrenTest );
        CPPUNIT_TEST( getObjectTest );
        CPPUNIT_TEST( getObjectDocumentTest );
        CPPUNIT_TEST( getByPathValidTest );
        CPPUNIT_TEST( getByPathInvalidTest );
        CPPUNIT_TEST( updatePropertiesTest );
        CPPUNIT_TEST( deleteObjectTest );
        CPPUNIT_TEST( getContentStreamTest );
        CPPUNIT_TEST_SUITE_END( );
};

void WSTest::getRepositoriesTest()
{
    list< libcmis::RepositoryPtr > actual = WSSession::getRepositories( SERVER_WSDL_URL, SERVER_USERNAME, SERVER_PASSWORD );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of repositories", size_t( 1 ), actual.size( ) );
}

void WSTest::sessionCreationTest( )
{
    WSSession session( SERVER_WSDL_URL, "", SERVER_USERNAME, SERVER_PASSWORD, false );
    CPPUNIT_ASSERT_MESSAGE( "No RepositoryService URL", !session.getServiceUrl( "RepositoryService" ).empty( ) );
}

void WSTest::getRepositoryTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Repository info badly retrieved", string( "100" ), session.getRepository()->getRootId( ) );
}

void WSTest::getRepositoryBadTest( )
{
    WSSession session( SERVER_WSDL_URL, "", SERVER_USERNAME, SERVER_PASSWORD, false );
    try
    {
        session.getRepositoryService( ).getRepositoryInfo( "bad" );
        CPPUNIT_FAIL( "Should have thrown SoapFault" );
    }
    catch( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception type", string( "invalidArgument" ), e.getType( ) );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception message",
                string( "Unknown repository id: bad" ), string( e.what( ) ) );
    }
}

void WSTest::getTypeDefinitionTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    string id( "ComplexType" ); 
    libcmis::ObjectTypePtr actual = session.getType( id );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong id", id, actual->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong parent type", string( "cmis:document" ), actual->getParentType( )->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong base type", string( "cmis:document" ), actual->getBaseType( )->getId( ) );
}

void WSTest::getTypeDefinitionErrorTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    
    string id( "bad_type" );
    try
    {
        session.getType( id );
        CPPUNIT_FAIL( "Exception should be raised: invalid ID" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception type", string( "objectNotFound" ), e.getType( ) );
        CPPUNIT_ASSERT_MESSAGE( "Empty exception message", !string( e.what() ).empty( ) );
    }
}

void WSTest::getTypeChildrenTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    libcmis::ObjectTypePtr actual = session.getType( "cmis:document" );

    vector< libcmis::ObjectTypePtr > children = actual->getChildren( );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of children imported", size_t( 10 ), children.size() );
}

void WSTest::getObjectTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    string id( "101" ); 
    libcmis::ObjectPtr actual = session.getObject( id );

    CPPUNIT_ASSERT_MESSAGE( "getTypeDescription doesn't work: properties are likely to be empty",
            NULL != actual->getTypeDescription( ).get( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong id", id, actual->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong base type", string( "cmis:folder" ), actual->getBaseType( ) );
    CPPUNIT_ASSERT_MESSAGE( "Not a libcmis::Folder instance",
            boost::dynamic_pointer_cast< libcmis::Folder >( actual ).get( ) != NULL );
    CPPUNIT_ASSERT( 0 != actual->getRefreshTimestamp( ) );
}

void WSTest::getObjectDocumentTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    string id( "114" ); 
    libcmis::ObjectPtr actual = session.getObject( id );

    CPPUNIT_ASSERT_MESSAGE( "getTypeDescription doesn't work: properties are likely to be empty",
            NULL != actual->getTypeDescription( ).get( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong id", id, actual->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong base type", string( "cmis:document" ), actual->getBaseType( ) );
    CPPUNIT_ASSERT_MESSAGE( "Not a libcmis::Document instance",
            boost::dynamic_pointer_cast< libcmis::Document >( actual ).get( ) != NULL );
    CPPUNIT_ASSERT( 0 != actual->getRefreshTimestamp( ) );
}

void WSTest::getByPathValidTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    try
    {
        libcmis::ObjectPtr object = session.getObjectByPath( "/My_Folder-0-0/My_Document-1-2" );

        CPPUNIT_ASSERT_MESSAGE( "Missing return object", object.get() );
    }
    catch ( const libcmis::Exception& e )
    {
        string msg = "Unexpected exception: ";
        msg += e.what();
        CPPUNIT_FAIL( msg.c_str() );
    }
}

void WSTest::getByPathInvalidTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    try
    {
        libcmis::ObjectPtr object = session.getObjectByPath( "/some/dummy/path" );
        CPPUNIT_FAIL( "Exception should be thrown: invalid Path" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong error type", string( "objectNotFound" ), e.getType() );
        CPPUNIT_ASSERT_MESSAGE( "Empty error message", !string( e.what() ).empty( ) );
    }
}

void WSTest::updatePropertiesTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    
    // Values for the test
    libcmis::ObjectPtr object = session.getObject( "114" );
    string propertyName( "cmis:name" ); 
    string expectedValue( "New name" );
    
    // Fill the map of properties to change
    map< string, libcmis::PropertyPtr > newProperties;

    libcmis::ObjectTypePtr objectType = object->getTypeDescription( );
    map< string, libcmis::PropertyTypePtr >::iterator it = objectType->getPropertiesTypes( ).find( propertyName );
    vector< string > values;
    values.push_back( expectedValue );
    libcmis::PropertyPtr property( new libcmis::Property( it->second, values ) );
    newProperties[ propertyName ] = property;

    // Update the properties (method to test)
    libcmis::ObjectPtr updated = object->updateProperties( newProperties );

    // Checks
    map< string, libcmis::PropertyPtr >::iterator propIt = updated->getProperties( ).find( propertyName );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong value after refresh", expectedValue, propIt->second->getStrings().front( ) );
}

void WSTest::deleteObjectTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );

    // Get the object to remove
    string id( "130" );
    libcmis::ObjectPtr object = session.getObject( id );

    // Remove the object (method to test)
    object->remove( false );

    // Check that the node doesn't exist anymore
    try
    {
        libcmis::ObjectPtr newObject = session.getObject( id );
        CPPUNIT_FAIL( "Should be removed, exception should have been thrown" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception type", string( "objectNotFound" ), e.getType( ) );
    }
}

void WSTest::deleteTreeTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );

    string id( "117" );
    libcmis::ObjectPtr object = session.getObject( id );
    libcmis::Folder* folder = dynamic_cast< libcmis::Folder* >( object.get() );
    CPPUNIT_ASSERT_MESSAGE( "Document to remove is missing", folder != NULL );

    folder->removeTree( );

    try
    {
        libcmis::ObjectPtr newObject = session.getObject( id );
        CPPUNIT_FAIL( "Should be removed, exception should have been thrown" );
    }
    catch ( const libcmis::Exception& e )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong exception message", string( "No such node: " + id ) , string( e.what() ) );
    }
}

void WSTest::getContentStreamTest( )
{
    WSSession session( SERVER_WSDL_URL, "A1", SERVER_USERNAME, SERVER_PASSWORD, false );
    libcmis::ObjectPtr object = session.getObject( "116" );
    libcmis::Document* document = dynamic_cast< libcmis::Document* >( object.get() );
    
    CPPUNIT_ASSERT_MESSAGE( "Document expected", document != NULL );

    try
    {
        boost::shared_ptr< istream >  is = document->getContentStream( );
        CPPUNIT_ASSERT_MESSAGE( "Content stream should be returned", NULL != is.get() );
        CPPUNIT_ASSERT_MESSAGE( "Non-empty content stream should be returned", is->good() && !is->eof() );
    }
    catch ( const libcmis::Exception& e )
    {
        string msg = "Unexpected exception: ";
        msg += e.what();
        CPPUNIT_FAIL( msg.c_str() );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( WSTest );
