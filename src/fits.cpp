#include "database.h"
#include "condition_parser.h"
#include "command_list.h"
#include "database_abstraction_layer.h"
#include "update_sanger.h"




//________________________________________________________________________________________________________________________________________________________________________________________________

int main(int argc, char *argv[]) {

    CommandLineParameterParser clp ( argc , argv ) ;
    if ( clp.command.empty() ) {
        cout << "USAGE: " << argv[0] << " COMMAND parameters" << endl ;
        return 1 ;
    }

    if ( clp.command == "list" ) {
        CommandList command ;
        if ( !command.parse(clp) ) {
            cerr << "PARSING ERROR: " << command.error_message << endl ;
            return 1 ;
        }
        if ( !command.run() ) {
            cerr << "RUN ERROR: " << command.error_message << endl ;
            return 1 ;
        }
    } else if ( clp.command == "update_sanger" ) {
        UpdateSanger ufs ;

        if ( clp.getParameterID("--pivot_views") != -1 ) { // ./fits update_sanger --pivot_views
            ufs.updatePivotView ( "file" ) ;
            ufs.updatePivotView ( "sample" ) ;
        } else {
            ufs.updateFromMLWH() ;
        }

    } else {
        cout << "Unknown command " << clp.command << endl ;
        return 1 ;
    }


/*    
    cout << "COMMAND: " << clp.command << endl ;
    for ( auto &p:clp.parameters ) {
        cout << p.key << ": " << p.value << endl ;
    }
*/

/*
    FileTrackingDatabase ft ;

    SQLresult r ;
    try {
	    r = ft.query ( "SELECT * FROM file LIMIT 5" ) ;
	} catch ( string s ) {
		cerr << s << endl ;
		return 1 ;
	} catch ( ... ) {
		cerr << "SQL query failed" << endl ;
		return 1 ;
	}

    cout << r.getNumRows() << " rows" << endl ;
    cout << r.getNumFields() << " fields" << endl ;
    SQLmap map ;
    while ( r.getMap(map) ) {
    	for ( auto &x:map ) cout << x.first << ":" << x.second << "; " ;
    	cout << endl ;
    }
*/
    return 0 ;
}
