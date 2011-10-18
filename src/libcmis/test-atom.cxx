#include <ctime>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/ui/text/TestRunner.h>

#include "atom-document.hxx"
#include "atom-folder.hxx"
#include "atom-session.hxx"
#include "atom-utils.hxx"

// InMemory local test server data
#define SERVER_ATOM_URL string( "http://localhost:8080/inmemory/atom" )
#define SERVER_REPOSITORIES_COUNT list< string >::size_type( 1 )
#define SERVER_REPOSITORY string( "A1" )
#define SERVER_USERNAME string( "tester" )
#define SERVER_PASSWORD string( "somepass" )

#define TEST_FOLDER_ID string( "101" )
#define TEST_FOLDER_NAME string( "My_Folder-0-0" )
#define TEST_FOLDER_PATH string( "/My_Folder-0-0" )

#define TEST_DOCUMENT_ID string( "116" )
#define TEST_DOCUMENT_NAME string( "My_Document-1-2" )
#define TEST_DOCUMENT_TYPE string( "text/plain" )

#define TEST_CHILDREN_FOLDER_COUNT 2
#define TEST_CHILDREN_DOCUMENT_COUNT 3
#define TEST_CHILDREN_COUNT vector<libcmis::ObjectPtr>::size_type( TEST_CHILDREN_FOLDER_COUNT + TEST_CHILDREN_DOCUMENT_COUNT )

using namespace boost;
using namespace std;

class AtomTest : public CppUnit::TestFixture
{
    public:

        void getRepositoriesTest( );
        void sessionCreationTest( );
        void getFolderCreationFromUrlTest( );
        void getDocumentCreationFromUrlTest( );
        void getChildrenTest( );
        void getContentTest( );
        void parseDateTimeTest( );

        CPPUNIT_TEST_SUITE( AtomTest );
        CPPUNIT_TEST( getRepositoriesTest );
        CPPUNIT_TEST( sessionCreationTest );
        CPPUNIT_TEST( getFolderCreationFromUrlTest );
        CPPUNIT_TEST( getDocumentCreationFromUrlTest );
        CPPUNIT_TEST( getChildrenTest );
        CPPUNIT_TEST( getContentTest );
        CPPUNIT_TEST( parseDateTimeTest );
        CPPUNIT_TEST_SUITE_END( );
};

void AtomTest::getRepositoriesTest()
{
    list< string > ids = AtomPubSession::getRepositories( SERVER_ATOM_URL, SERVER_USERNAME, SERVER_PASSWORD );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "One repository should be found", SERVER_REPOSITORIES_COUNT, ids.size() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong repository found", SERVER_REPOSITORY, ids.front() );
}

void AtomTest::sessionCreationTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD);

    // Check for the mandatory collection URLs
    CPPUNIT_ASSERT_MESSAGE( "root collection URL missing",
            !session.getCollectionUrl( Collection::Root ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "types collection URL missing",
            !session.getCollectionUrl( Collection::Types ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "query collection URL missing",
            !session.getCollectionUrl( Collection::Query ).empty() );

    // The optional collection URLs are present on InMemory, so check them
    CPPUNIT_ASSERT_MESSAGE( "checkedout collection URL missing",
            !session.getCollectionUrl( Collection::CheckedOut ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "unfiled collection URL missing",
            !session.getCollectionUrl( Collection::Unfiled ).empty() );

    // Check for the mandatory URI template URLs
    CPPUNIT_ASSERT_MESSAGE( "objectbyid URI template URL missing",
            !session.getUriTemplate( UriTemplate::ObjectById ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "objectbypath URI template URL missing",
            !session.getUriTemplate( UriTemplate::ObjectByPath ).empty() );
    CPPUNIT_ASSERT_MESSAGE( "typebyid URI template URL missing",
            !session.getUriTemplate( UriTemplate::TypeById ).empty() );
    
    // The optional URI template URL is present on InMemory, so check it
    CPPUNIT_ASSERT_MESSAGE( "query URI template URL missing",
            !session.getUriTemplate( UriTemplate::Query ).empty() );

    // Check that the root id is defined
    CPPUNIT_ASSERT_MESSAGE( "Root node ID is missing",
            !session.getRootId().empty() );
}

void AtomTest::getFolderCreationFromUrlTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );
    libcmis::FolderPtr folder = session.getFolder( TEST_FOLDER_ID );

    AtomFolder* atomFolder = dynamic_cast< AtomFolder* >( folder.get( ) );
    CPPUNIT_ASSERT_MESSAGE( "Created folder should be an instance of AtomFolder",
            NULL != atomFolder );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong folder ID", TEST_FOLDER_ID, folder->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong folder name", TEST_FOLDER_NAME, folder->getName( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong folder path", TEST_FOLDER_PATH, folder->getPath( ) );
    CPPUNIT_ASSERT_MESSAGE( "Children URL is missing", !atomFolder->getChildrenUrl( ).empty( ) );

    CPPUNIT_ASSERT_MESSAGE( "CreatedBy is missing", !atomFolder->getCreatedBy( ).empty( ) );
    CPPUNIT_ASSERT_MESSAGE( "CreationDate is missing", !atomFolder->getCreationDate( ).is_not_a_date_time() );
    CPPUNIT_ASSERT_MESSAGE( "LastModifiedBy is missing", !atomFolder->getLastModifiedBy( ).empty( ) );
    CPPUNIT_ASSERT_MESSAGE( "LastModificationDate is missing", !atomFolder->getLastModificationDate( ).is_not_a_date_time() );
    CPPUNIT_ASSERT_MESSAGE( "ChangeToken is missing", !atomFolder->getChangeToken( ).empty( ) );
}

void AtomTest::getDocumentCreationFromUrlTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );
    libcmis::ObjectPtr object = session.getObject( TEST_DOCUMENT_ID );

    AtomDocument* atomDocument = dynamic_cast< AtomDocument* >( object.get( ) );
    CPPUNIT_ASSERT_MESSAGE( "Fetched object should be an instance of AtomDocument",
            NULL != atomDocument );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong document ID", TEST_DOCUMENT_ID, atomDocument->getId( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong document name", TEST_DOCUMENT_NAME, atomDocument->getName( ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong document type", TEST_DOCUMENT_TYPE, atomDocument->getContentType( ) );

    CPPUNIT_ASSERT_MESSAGE( "CreatedBy is missing", !atomDocument->getCreatedBy( ).empty( ) );
    CPPUNIT_ASSERT_MESSAGE( "CreationDate is missing", !atomDocument->getCreationDate( ).is_not_a_date_time() );
    CPPUNIT_ASSERT_MESSAGE( "LastModifiedBy is missing", !atomDocument->getLastModifiedBy( ).empty( ) );
    CPPUNIT_ASSERT_MESSAGE( "LastModificationDate is missing", !atomDocument->getLastModificationDate( ).is_not_a_date_time() );
    CPPUNIT_ASSERT_MESSAGE( "ChangeToken is missing", !atomDocument->getChangeToken( ).empty( ) );

    // Don't test the exact value... the content is changing at each restart of the InMemory server
    CPPUNIT_ASSERT_MESSAGE( "Content length is missing", 0 < atomDocument->getContentLength( ) );
}

void AtomTest::getChildrenTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );
    libcmis::FolderPtr folder = session.getRootFolder( );

    vector< libcmis::ObjectPtr > children = folder->getChildren( );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of children", TEST_CHILDREN_COUNT, children.size() );

    int folderCount = 0;
    int documentCount = 0;
    for ( vector< libcmis::ObjectPtr >::iterator it = children.begin( );
          it != children.end( ); it++ )
    {
        if ( NULL != dynamic_cast< AtomFolder* >( it->get() ) )
            ++folderCount;
        else if ( NULL != dynamic_cast< AtomDocument* >( it->get() ) )
            ++documentCount;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of folder children",
            TEST_CHILDREN_FOLDER_COUNT, folderCount );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of document children",
            TEST_CHILDREN_DOCUMENT_COUNT, documentCount );
}

void AtomTest::getContentTest( )
{
    AtomPubSession session( SERVER_ATOM_URL, SERVER_REPOSITORY, SERVER_USERNAME, SERVER_PASSWORD );
    libcmis::ObjectPtr object = session.getObject( TEST_DOCUMENT_ID );
    libcmis::Document* document = dynamic_cast< libcmis::Document* >( object.get() );
    
    CPPUNIT_ASSERT_MESSAGE( "Document expected", document != NULL );

    FILE* fd = document->getContent( );
    CPPUNIT_ASSERT_MESSAGE( "Temporary file with content should be returned", NULL != fd );
}

void AtomTest::parseDateTimeTest( )
{
    tm basis;
    basis.tm_year = 2011 - 1900;
    basis.tm_mon = 8; // Months are in 0..11 range
    basis.tm_mday = 28;
    basis.tm_hour = 12;
    basis.tm_min = 44;
    basis.tm_sec = 28;

    // No time zone test
    {
        char toParse[50];
        strftime( toParse, sizeof( toParse ), "%FT%T", &basis );
        posix_time::ptime t = atom::parseDateTime( string( toParse ) );

        gregorian::date expDate( basis.tm_year + 1900, basis.tm_mon + 1, basis.tm_mday );
        posix_time::time_duration expTime( basis.tm_hour, basis.tm_min, basis.tm_sec );
        posix_time::ptime expected( expDate, expTime );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "No time zone case failed", expected, t );
    }
    
    // Z time zone test
    {
        char toParse[50];
        strftime( toParse, sizeof( toParse ), "%FT%TZ", &basis );
        posix_time::ptime t = atom::parseDateTime( string( toParse ) );
        
        gregorian::date expDate( basis.tm_year + 1900, basis.tm_mon + 1, basis.tm_mday );
        posix_time::time_duration expTime( basis.tm_hour, basis.tm_min, basis.tm_sec );
        posix_time::ptime expected( expDate, expTime );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Z time zone case failed", expected, t );
    }

    // +XX:XX time zone test
    {
        char toParse[50];
        strftime( toParse, sizeof( toParse ), "%FT%T+02:00", &basis );
        posix_time::ptime t = atom::parseDateTime( string( toParse ) );
        
        gregorian::date expDate( basis.tm_year + 1900, basis.tm_mon + 1, basis.tm_mday );
        posix_time::time_duration expTime( basis.tm_hour + 2, basis.tm_min, basis.tm_sec );
        posix_time::ptime expected( expDate, expTime );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "+XX:XX time zone case failed", expected, t );
    }

    // -XX:XX time zone test
    {
        char toParse[50];
        strftime( toParse, sizeof( toParse ), "%FT%T-02:00", &basis );
        posix_time::ptime t = atom::parseDateTime( string( toParse ) );
        
        gregorian::date expDate( basis.tm_year + 1900, basis.tm_mon + 1, basis.tm_mday );
        posix_time::time_duration expTime( basis.tm_hour - 2, basis.tm_min, basis.tm_sec );
        posix_time::ptime expected( expDate, expTime );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "+XX:XX time zone case failed", expected, t );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( AtomTest );

int main( int argc, char* argv[] )
{
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    bool wasSuccess = runner.run( "", false );
    return !wasSuccess;
}
