#ifndef _ATOM_UTILS_HXX_
#define _ATOM_UTILS_HXX_

#include <string>

#include <libxml/xpathInternals.h>

#define NS_APP_URL BAD_CAST( "http://www.w3.org/2007/app" )
#define NS_ATOM_URL BAD_CAST( "http://www.w3.org/2005/Atom" )
#define NS_CMIS_URL BAD_CAST( "http://docs.oasis-open.org/ns/cmis/core/200908/" )
#define NS_CMISRA_URL BAD_CAST( "http://docs.oasis-open.org/ns/cmis/restatom/200908/" )

#define URI_TEMPLATE_VAR_ID std::string( "id" )

namespace atom
{
    void registerNamespaces( xmlXPathContextPtr pXPathCtx );

    std::string getXPathValue( xmlXPathContextPtr pXPathCtx, std::string req ); 

    void http_request( std::string Url,
            size_t (*pCallback)( void *, size_t, size_t, void* ),
            void* pData );
}

#endif
