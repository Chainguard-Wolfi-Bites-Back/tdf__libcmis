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

#include <stdio.h>

#include <exception>
#include <iostream>
#include <map>
#include <string>

#include <boost/program_options.hpp>

#include <libcmis/session-factory.hxx>
#include <libcmis/content.hxx>

using namespace std;
using namespace ::boost::program_options;

class CommandException : public exception
{
    private:
        const char* m_msg;

    public:
        CommandException( const char* msg ) : m_msg( msg ) { }
        virtual const char* what() const throw() { return m_msg; }
};

class CmisClient
{
    private:
        variables_map& m_vm;
    public:
        CmisClient( variables_map& vm ) : m_vm( vm ) { }

        Session* getSession( ) throw ( CommandException );

        void execute( ) throw ( exception );

        void printHelp( );

        static options_description getOptionsDescription( );
};

Session* CmisClient::getSession( ) throw ( CommandException )
{
    map< int, string > params;

    if ( m_vm.count( "url" ) == 0 )
        throw CommandException( "Missing Atom binding URL" );
    
    string url = m_vm["url"].as<string>();
    params[ATOMPUB_URL] = url;
    list< string > ids = SessionFactory::getRepositories( params );

    // The repository ID is needed to initiate a session
    if ( m_vm.count( "repository" ) != 1 )
        throw CommandException( "Missing repository ID" );

    // Get the ids of the objects to fetch
    if ( m_vm.count( "args" ) == 0 )
        throw CommandException( "Please provide the node ids to show as command args" );

    params[REPOSITORY_ID] = m_vm["repository"].as< string >();
    return SessionFactory::createSession( params );
}

void CmisClient::execute( ) throw ( exception )
{
    if ( ( m_vm.count( "help" ) > 0 ) || m_vm.count( "command" ) != 1 )
    {
        printHelp();
        return; 
    }

    if ( m_vm.count( "command" ) == 1 )
    {
        string command = m_vm["command"].as<string>();
        if ( "list-repos" == command )
        {
            map< int, string > params;
            if ( m_vm.count( "url" ) == 0 )
                throw CommandException( "Missing URL" );
            string url = m_vm["url"].as<string>();

            params[ATOMPUB_URL] = url;
            list< string > ids = SessionFactory::getRepositories( params );
        
            cout << "Repositories: ";
            for ( list< string >::iterator it = ids.begin(); it != ids.end(); it++ )
            {
                if ( it != ids.begin() )
                    cout << ", ";
                cout << *it;
            }
            cout << endl;
        }
        else if ( "show-by-id" == command )
        {
            Session* session = getSession( );

            vector< string > objIds = m_vm["args"].as< vector< string > >( );


            for ( vector< string >::iterator it = objIds.begin(); it != objIds.end(); it++ )
            {
                CmisObjectPtr cmisObj = session->getObject( *it );
                cout << "-----------------------" << endl;
                cout << cmisObj->toString() << endl;
            }

            delete session;
        }
        else if ( "get-content" == command )
        {
            Session* session = getSession( );

            vector< string > objIds = m_vm["args"].as< vector< string > >( );
            if ( objIds.size() == 0 )
                throw CommandException( "Please provide a content object Id" );

            CmisObjectPtr cmisObj = session->getObject( objIds.front() );
            Content* content = dynamic_cast< Content* >( cmisObj.get() );
            if ( NULL != content )
            {
                // TODO Handle name clashes
                FILE* fd = fopen( content->getContentFilename().c_str(), "w" );
                content->getContent( (size_t (*)( void*, size_t, size_t, void* ) )fwrite, fd );
                fclose( fd );
            }

            delete session;
        }
        else if ( "help" == command )
        {
            printHelp();
        }

        // TODO Add some more useful commands here
    }
}

options_description CmisClient::getOptionsDescription( )
{
    options_description desc( "Allowed options" );
    desc.add_options( )
        ( "help", "Produce help message and exists" )
        ( "url,u", value< string >(), "URL of the binding of the server" )
        ( "repository,r", value< string >(), "Name of the repository to use" )
    ;

    return desc;
}

void CmisClient::printHelp( )
{
    cerr << "CmisClient [options] [command] arguments" << endl;

    cerr << endl << "Commands" << endl;
    cerr << "   list-repos\n"
            "           lists the repositories available on the server" << endl;
    cerr << "   show-by-id <Node Id 1> [... <Node Id N>]\n"
            "           Dump the nodes informations for all the ids." << endl;
    cerr << "   get-content <Node Id>\n"
            "           Saves the stream of the content node in the\n"
            "           current folder. Any existing file is overwritten." << endl;
    cerr << "   help\n"
            "           Print this help message and exits (like --help option)." << endl;

    cerr << endl << getOptionsDescription() << endl;
}

int main ( int argc, char* argv[] )
{
    options_description hidden( "Hidden options" );
    hidden.add_options( )
        ( "command", value< string >(), "Command" )
        ( "args", value< vector< string > >(), "Arguments for the command" )
    ;

    options_description allOptions = CmisClient::getOptionsDescription( );
    allOptions.add( hidden );

    positional_options_description pd;
    pd.add( "command", 1 );
    pd.add( "args", -1 );

    variables_map vm;
    store( command_line_parser( argc, argv ).options( allOptions ).positional( pd ).run( ), vm );
    notify( vm );

    CmisClient client( vm );
    try
    {
        client.execute( );
    }
    catch ( CommandException e )
    {
        cerr << "-------------------------" << endl;
        cerr << "ERROR: " << e.what() << endl;
        cerr << "-------------------------" << endl;
        client.printHelp();
    }

    return 0;
}
