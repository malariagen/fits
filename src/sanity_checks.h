#ifndef __SANITY_CHECKS_H__
#define __SANITY_CHECKS_H__

#include "database_abstraction_layer.h"

class SanityChecks : public StringTools {
public:
    SanityChecks () ;

protected:
    void checkTagCounts ( string table , string tag_id , string tag_name , string min , string max ) ;
    DatabaseAbstractionLayer dab ;
} ;

#endif
