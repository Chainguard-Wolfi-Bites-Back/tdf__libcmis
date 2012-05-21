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
#ifndef _DOCUMENT_HXX_
#define _DOCUMENT_HXX_

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "exception.hxx"
#include "object.hxx"

namespace libcmis
{
    class Folder;

    /** Interface for a CMIS Document object.
      */
    class Document : public virtual Object
    {
        public:

            virtual ~Document( ) { }

            /** Get the folder parents for the document.

                Note that an unfiled document will have no parent folder.

                @return the parents folder if any.
              */
            virtual std::vector< boost::shared_ptr< Folder > > getParents( ) throw ( Exception ) = 0;
            
            /** Get the content stream or NULL if there is none.

                @param path
                    Save the stream to the given path. If the path
                    is NULL, then use a temporary file.

                @return
                    An opened stream to the content, or NULL if
                    something wrong happened during the download.

                @deprecated
                    This method is kept only for compatibility with
                    previous versions and is likely to be removed soon.

                @see Document::getContentStream()
              */
            virtual FILE* getContent( const char* path = NULL ) = 0;

            /** Get the content stream without using a temporary file.

                <p>The stream may not contain anything if there is
                no content or if something wrong happened during the
                download.</p>

                @return
                    An input stream to read the data from.

                @throws Exception
                    if anything wrong happened during the file transfer.
                    In such a case, the content of the stream can't be
                    guaranteed.
              */
            virtual boost::shared_ptr< std::istream > getContentStream( ) throw ( Exception ) = 0;

            /** Set or replace the content stream of the document.

                @param is the output stream containing the new data for the content stream
                @param contentType the mime-type of the new content stream
                @param overwrite if set to false, don't overwrite the content stream if one is already set.

                @throw Exception if anything happens during the upload like a wrong authentication, 
                                no rights to set the stream, server doesn't have the ContentStreamUpdatability
                                capability.
              */
            virtual void setContentStream( std::ostream& os, std::string contentType,
                                           bool overwrite = true ) throw ( Exception ) = 0;

            /** Get the content mime type.
              */
            virtual std::string getContentType( ) = 0;
            
            /** Get the content stream filename.
              */
            virtual std::string getContentFilename( ) = 0;

            /** Get the content length in bytes.
              */
            virtual long getContentLength( ) = 0;
    };
}

#endif
