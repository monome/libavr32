#include "compiler.h"
#include "types.h"

#include "kbd.h"


bool frame_compare(u8 data) {
	u8 i;
	for( i = 2; i < 8; i++ )
		if( old_frame[ i ] == data )
			return( true );

 	return( false );
}

extern void set_old_frame(const s8 *data) {
	u8 i;
	for(i = 2; i<8; i++)
		old_frame[i] = data[i];
}


u8 hid_to_ascii(u8 data, u8 mod) {
  	/* upper row of the keyboard, numbers and special symbols */
	if( data >= 0x1e && data <= 0x27 ) {
		// shift key pressed
	    if(mod & SHIFT) {
	    	switch( data ) {
	    		case BANG:  return( 0x21 );
	    		case AT:    return( 0x40 );
	    		case POUND: return( 0x23 );
	    		case DOLLAR: return( 0x24 );
	    		case PERCENT: return( 0x25 );
	    		case CAP: return( 0x5e );
	    		case AND: return( 0x26 );
	    		case STAR: return( 0x2a );
	    		case OPENBKT: return( 0x28 );
	    		case CLOSEBKT: return( 0x29 );
			}
		}
		// numbers
	    else {
			if( data == 0x27 ) {  //zero
		      	return( 0x30 );
		    }
		    else {
	      		return( data + 0x13 );
		    }
	    }
	}

	/* number pad. Arrows are not supported */
	// if(( data >= 0x59 && data <= 0x61 ) && ( numLock == true )) {  // numbers 1-9
	// 	return( data - 0x28 );
	// }
	// if(( data == 0x62 ) && ( numLock == true )) {                      //zero
	// 	return( 0x30 );
	// }

	/* Letters a-z */
	if( data >= 0x04 && data <= 0x1d ) {
		// if((( capsLock == true ) && ( mod & SHIFT ) == 0 ) || (( capsLock == false ) && ( mod & SHIFT ))) {  //upper case
			return( data + 0x3d );
		// }
		// else {  //lower case
		// 	return( data + 0x5d );
		// }
	}

	/* Other special symbols */
  	if( data >= 0x2c && data <= 0x38 ) {
  		switch( data ) {
  			case SPACE: return( 0x20 ); 
  			case HYPHEN:
	  			if(( mod & SHIFT ) == false ) {
	  				return( 0x2d );
	  			}
	  			else {
	  				return( 0x5f );
	  			}
  			case EQUAL:
	  			if(( mod & SHIFT ) == false ) {
	  				return( 0x3d );
	  			}
	  			else {
	  				return( 0x2b );
	  			}
  			case SQBKTOPEN:
	  			if(( mod & SHIFT ) == false ) {
	  				return( 0x5b );
	  			}
	  			else {
	  				return( 0x7b );
	  			}
  			case SQBKTCLOSE:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x5d );
	  			}
	  			else {
	  				return( 0x7d );
	  			} 
  			case BACKSLASH:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x5c );
	  			}
	  			else {
	  				return( 0x7c );
	  			}
  			case SEMICOLON:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x3b );
	  			}
	  			else {
	  				return( 0x3a );
	  			}
  			case INVCOMMA:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x27 );
	  			}
	  			else {
	  				return( 0x22 );
	  			}
  			case TILDE:  
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x60 );
	  			}
	  			else {
	  				return( 0x7e );
	  			}
  			case COMMA:   
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x2c );
	  			}
	  			else {
	  				return( 0x3c );
	  			}
  			case PERIOD:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x2e );
	  			}
	  			else {
	  				return( 0x3e );
	  			}
  			case FRONTSLASH:
  			if(( mod & SHIFT ) == false ) {
	  				return( 0x2f );
	  			}
	  			else {
	  				return( 0x3f );
	  			}
  			default:
	  			break;
    	}
	}
  	
  	return( 0 );
}