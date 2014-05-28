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
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>

#include <string>

#define private public
#define protected public

#include <mockup-config.h>

#include <fstream>
#include "onedrive-property.hxx"
#include "onedrive-session.hxx"
#include "oauth2-handler.hxx"
#include "document.hxx"

using namespace std;
using namespace libcmis;

static const string CLIENT_ID ( "mock-id" );
static const string CLIENT_SECRET ( "mock-secret" );
static const string USERNAME( "mock-user" );
static const string PASSWORD( "mock-password" );
static const string LOGIN_URL ("https://login/url" );
static const string APPROVAL_URL ("https://approval/url" );
static const string AUTH_URL ( "https://auth/url" );
static const string TOKEN_URL ( "https://token/url" );
static const string SCOPE ( "https://scope/url" );
static const string REDIRECT_URI ("redirect:uri" );
static const string BASE_URL ( "https://base/url" );

class OneDriveTest : public CppUnit::TestFixture
{
    public:
        void sessionAuthenticationTest( );
        void sessionExpiryTokenGetTest( );
        void getRepositoriesTest( );
        void filePropertyTest( );
        void folderListedPropertyTest( );

        CPPUNIT_TEST_SUITE( OneDriveTest );
        CPPUNIT_TEST( sessionAuthenticationTest );
        CPPUNIT_TEST( sessionExpiryTokenGetTest );
        CPPUNIT_TEST( getRepositoriesTest );
        CPPUNIT_TEST( filePropertyTest );
        CPPUNIT_TEST( folderListedPropertyTest );
        CPPUNIT_TEST_SUITE_END( );

    private:
        OneDriveSession getTestSession( string username, string password );
};

OneDriveSession OneDriveTest::getTestSession( string username, string password )
{
    libcmis::OAuth2DataPtr oauth2(
        new libcmis::OAuth2Data( AUTH_URL, TOKEN_URL, SCOPE,
                                 REDIRECT_URI, CLIENT_ID, CLIENT_SECRET ));
    curl_mockup_reset( );
    string empty;
    // login, authentication & approval are done manually at the moment, so I'll
    // temporarily borrow them from gdrive
    //login response
    string loginIdentifier = string("scope=") + SCOPE +
                             string("&redirect_uri=") + REDIRECT_URI +
                             string("&response_type=code") +
                             string("&client_id=") + CLIENT_ID;
    curl_mockup_addResponse ( AUTH_URL.c_str(), loginIdentifier.c_str( ),
                            "GET", DATA_DIR "/gdrive/login.html", 200, true);

    //authentication response
    curl_mockup_addResponse( LOGIN_URL.c_str( ), empty.c_str( ), "POST",
                             DATA_DIR "/gdrive/approve.html", 200, true);

    //approval response
    curl_mockup_addResponse( APPROVAL_URL.c_str( ), empty.c_str( ),
                             "POST", DATA_DIR "/gdrive/authcode.html", 200, true);


    // token response
    curl_mockup_addResponse ( TOKEN_URL.c_str( ), empty.c_str( ), "POST",
                              DATA_DIR "/onedrive/token-response.json", 200, true );

    return OneDriveSession( BASE_URL, username, password, oauth2, false );
}

void OneDriveTest::sessionAuthenticationTest( )
{
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );
    string empty;

    // Check token request
    string expectedTokenRequest =
        string( "code=AuthCode") +
        string( "&client_id=") + CLIENT_ID +
        string( "&client_secret=") + CLIENT_SECRET +
        string( "&redirect_uri=") + REDIRECT_URI +
        string( "&grant_type=authorization_code" );

    string tokenRequest( curl_mockup_getRequestBody( TOKEN_URL.c_str(), empty.c_str( ),
                                                 "POST" ) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong token request",
                                  expectedTokenRequest, tokenRequest );

    // Check token
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong access token",
         string ( "mock-access-token" ),
         session.m_oauth2Handler->getAccessToken( ));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong refresh token",
        string ("mock-refresh-token"),
        session.m_oauth2Handler->getRefreshToken( ));
}

void OneDriveTest::sessionExpiryTokenGetTest( )
{
    // Access_token will expire after expires_in seconds,
    // We need to use the refresh key to get a new one.

    curl_mockup_reset( );
    OneDriveSession session = getTestSession( USERNAME, PASSWORD );

    curl_mockup_reset( );
    static const string objectId("aFileId");
    string url = BASE_URL + "/me/skydrive/files/" + objectId;

    // 401 response, token is expired
    curl_mockup_addResponse( url.c_str( ),"", "GET", "", 401, false );

    curl_mockup_addResponse( TOKEN_URL.c_str(), "",
                             "POST", DATA_DIR "/onedrive/refresh-response.json", 200, true);
    try
    {
        // GET expires, need to refresh then GET again
        libcmis::ObjectPtr obj = session.getObject( objectId );
    }
    catch ( ... )
    {
        if ( session.getHttpStatus( ) == 401 )
        {
            // Check if access token is refreshed
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                   "wrong access token",
                   string ( "new-access-token" ),
                   session.m_oauth2Handler->getAccessToken( ) );
        }
    }
}

void OneDriveTest::getRepositoriesTest( )
{
     curl_mockup_reset( );

     OneDriveSession session = getTestSession( USERNAME, PASSWORD );
     vector< libcmis::RepositoryPtr > actual = session.getRepositories( );

     CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong number of repositories", size_t( 1 ),
                                   actual.size( ) );
     CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong repository found",
                                   string ( "OneDrive" ),
                                   actual.front()->getId( ) );
}

void OneDriveTest::filePropertyTest( )
{
    static const string objectId( "aFileId" );
    static string objectPath = DATA_DIR;
    objectPath += "/onedrive/file.json";
    std::ifstream fin( objectPath.c_str( ) );
    std::stringstream res;
    res << fin.rdbuf( );

    Json jsonRes = Json::parse( res.str( ) );
    Json::JsonObject objs = jsonRes.getObjects( );
    Json::JsonObject::iterator it;
    for ( it = objs.begin( ); it != objs.end( ); ++it)
    {
        PropertyPtr property;
        property.reset( new OneDriveProperty( it->first, it->second ) );
        const string localName = property->getPropertyType( )->getLocalName( );
        const string propValue = property->toString( );

        if (localName == "cmis:creationDate")
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong creation date",
                                           string ( "createdTime" ),
                                           propValue );
        }
        else if (localName == "cmis:objectId")
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong object id",
                                           string ( "aFileId" ),
                                           propValue );
        }
        else if (localName == "cmis:createdBy")
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong author",
                                           string ( "onedriveUser" ),
                                           propValue );
        }
        else if (localName == "cmis:contentStreamFileName")
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong file name",
                                           string ( "OneDrive File" ),
                                           propValue );
        }

    }
}

void OneDriveTest::folderListedPropertyTest( )
{
    static const string objectId( "aFileId" );
    static string objectPath = DATA_DIR;
    objectPath += "/onedrive/folder-listed.json";
    std::ifstream fin( objectPath.c_str( ) );
    std::stringstream res;
    res << fin.rdbuf( );

    Json jsonRes = Json::parse( res.str( ) );
    Json::JsonVector objsList = jsonRes["data"].getList( );

    for(unsigned int i = 0; i < objsList.size(); i++)
    {
        Json::JsonObject objs = objsList[i].getObjects( );
        Json::JsonObject::iterator it;

        for ( it = objs.begin( ); it != objs.end( ); ++it )
        {
            PropertyPtr property;
            property.reset( new OneDriveProperty( it->first, it->second ) );
            const string localName = property->getPropertyType( )->getLocalName( );
            const string propValue = property->toString( );

            if (localName == "cmis:creationDate")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong creation date",
                                               string ( "createdTime" ),
                                               propValue );
            }
            else if (localName == "cmis:createdBy")
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong author",
                                               string ( "onedriveUser" ),
                                               propValue );
            }
            if ( objs["type"].toString( ) == "file" )
            {
                if (localName == "cmis:objectId")
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong object id",
                                                   string ( "aFileId" ),
                                                   propValue );
                }
                
                else if (localName == "cmis:contentStreamFileName")
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong file name",
                                                   string ( "OneDrive File" ),
                                                   propValue );
                }
            }
            else
            {
                if (localName == "cmis:objectId")
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong object id",
                                                   string ( "aFolderId" ),
                                                   propValue );
                }
                
                else if (localName == "cmis:contentStreamFileName")
                {
                    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Wrong folder name",
                                                   string ( "OneDrive Folder" ),
                                                   propValue );
                }
            }
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION( OneDriveTest );
