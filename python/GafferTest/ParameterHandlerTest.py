##########################################################################
#  
#  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#  
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#  
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#  
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
##########################################################################

import unittest

import IECore

import Gaffer

class ParameterHandlerTest( unittest.TestCase ) :

	def testFactory( self ) :

		p = IECore.IntParameter( "i", "d", 10 )
		
		n = Gaffer.Node()
		h = Gaffer.ParameterHandler.create( p, n )
		
		self.failUnless( isinstance( h, Gaffer.ParameterHandler ) )
		self.failUnless( isinstance( n["i"], Gaffer.IntPlug ) )
		
	def testCustomHandler( self ) :
	
		class CustomParameter( IECore.IntParameter ) :
		
			def __init__( self, name, description, defaultValue ) :
			
				IECore.IntParameter.__init__( self, name, description, defaultValue )
		
		IECore.registerRunTimeTyped( CustomParameter )
				
		class CustomHandler( Gaffer.ParameterHandler ) :
		
			def __init__( self, parameter, plugParent ) :
						
				Gaffer.ParameterHandler.__init__( self, parameter )
				
				self.__plug = plugParent.getChild( parameter.name )
				if not isinstance( self.__plug, Gaffer.IntPlug ) :
					self.__plug = Gaffer.IntPlug(
						parameter.name,
						Gaffer.Plug.Direction.In,
						parameter.numericDefaultValue,
						parameter.minValue,
						parameter.maxValue
					)
					
				plugParent[parameter.name] = self.__plug
				
			def setParameterValue( self ) :
			
				self.parameter().setValue( self.__plug.getValue() * 10 )
				
			def setPlugValue( self ) :
							
				self.__plug.setValue( self.parameter().getNumericValue() / 10 )
						
		Gaffer.ParameterHandler.registerParameterHandler( CustomParameter.staticTypeId(), CustomHandler )
		
		p = IECore.Parameterised( "" )
		p.parameters().addParameter(
			
			CustomParameter( 
				
				"i",
				"d",
				10
			
			)
		
		)
		
		ph = Gaffer.ParameterisedHolderNode()
		ph.setParameterised( p )
		
		self.assertEqual( ph["parameters"]["i"].getValue(), 1 )
		
		with ph.parameterModificationContext() as parameters :
		
			p["i"].setNumericValue( 100 )
			
		self.assertEqual( ph["parameters"]["i"].getValue(), 10 )
		
		ph["parameters"]["i"].setValue( 1000 )
		
		ph.setParameterisedValues()
		
		self.assertEqual( p["i"].getNumericValue(), 10000 )
		
if __name__ == "__main__":
	unittest.main()
	