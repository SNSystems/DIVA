#include <stdio.h>
#include <assert.h>

class Exception {
	public:
		Exception( int i ) : error(i) {};
		
		void caught() { fprintf( stderr, "%d\n", error ); }
	
	protected:
		int error;
	};

class ExceptionThrower {

	public:
		ExceptionThrower() {};

		void throwException() { throw Exception( 7 ); }
	};

int main( int argc, char ** argv ) {
	ExceptionThrower et = ExceptionThrower();
	
	try{ et.throwException(); }
	catch( Exception & e ) {
		e.caught();
		}

	return 0;
	} /* end main() */
