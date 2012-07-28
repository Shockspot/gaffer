//////////////////////////////////////////////////////////////////////////
//  
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//  
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//  
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//  
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//////////////////////////////////////////////////////////////////////////

#include "IECore/BoxOps.h"
#include "IECore/BoxAlgo.h"

#include "GafferImage/ImagePrimitiveSource.h"

namespace GafferImage
{

template<typename BaseType>
const IECore::RunTimeTyped::TypeDescription<ImagePrimitiveSource<BaseType> > ImagePrimitiveSource<BaseType>::g_typeDescription;

template<typename BaseType>
ImagePrimitiveSource<BaseType>::ImagePrimitiveSource( const std::string &name )
	:	BaseType( name )
{
	BaseType::addChild( new Gaffer::ObjectPlug( "__imagePrimitive", Gaffer::Plug::Out ) );
	BaseType::addChild( new Gaffer::ObjectPlug( "__inputImagePrimitive", Gaffer::Plug::In, 0, Gaffer::Plug::Default & ~Gaffer::Plug::Serialisable ) );
	inputImagePrimitivePlug()->setInput( imagePrimitivePlug() );
}

template<typename BaseType>
ImagePrimitiveSource<BaseType>::~ImagePrimitiveSource()
{
}

template<typename BaseType>
void ImagePrimitiveSource<BaseType>::affects( const Gaffer::ValuePlug *input, Gaffer::Node::AffectedPlugsContainer &outputs ) const
{
	BaseType::affects( input, outputs );
	
	if( input == inputImagePrimitivePlug() )
	{
		outputs.push_back( BaseType::outPlug() );
	}
}

template<typename BaseType>
Gaffer::ObjectPlug *ImagePrimitiveSource<BaseType>::imagePrimitivePlug()
{
	return BaseType::template getChild<Gaffer::ObjectPlug>( "__imagePrimitive" );
}

template<typename BaseType>
const Gaffer::ObjectPlug *ImagePrimitiveSource<BaseType>::imagePrimitivePlug() const
{
	return BaseType::template getChild<Gaffer::ObjectPlug>( "__imagePrimitive" );
}

template<typename BaseType>
Gaffer::ObjectPlug *ImagePrimitiveSource<BaseType>::inputImagePrimitivePlug()
{
	return BaseType::template getChild<Gaffer::ObjectPlug>( "__inputImagePrimitive" );
}

template<typename BaseType>
const Gaffer::ObjectPlug *ImagePrimitiveSource<BaseType>::inputImagePrimitivePlug() const
{
	return BaseType::template getChild<Gaffer::ObjectPlug>( "__inputImagePrimitive" );
}
		
template<typename BaseType>
void ImagePrimitiveSource<BaseType>::compute( Gaffer::ValuePlug *output, const Gaffer::Context *context ) const
{
	if( output == imagePrimitivePlug() )
	{
		static_cast<Gaffer::ObjectPlug *>( output )->setValue( computeImagePrimitive( context ) );
		return;
	}
	
	return BaseType::compute( output, context );
}

template<typename BaseType>
Imath::Box2i ImagePrimitiveSource<BaseType>::computeDisplayWindow( const Gaffer::Context *context, const ImagePlug *parent ) const
{
	Imath::Box2i result;
	IECore::ConstImagePrimitivePtr image = IECore::staticPointerCast<const IECore::ImagePrimitive>( inputImagePrimitivePlug()->getValue() );
	if( image )
	{
		result = image->getDisplayWindow();
	}
	return result;
}

template<typename BaseType>
Imath::Box2i ImagePrimitiveSource<BaseType>::computeDataWindow( const Gaffer::Context *context, const ImagePlug *parent ) const
{
	Imath::Box2i result;
	IECore::ConstImagePrimitivePtr image = IECore::staticPointerCast<const IECore::ImagePrimitive>( inputImagePrimitivePlug()->getValue() );
	if( image )
	{
		result = image->getDataWindow();
	}
	return result;
}

template<typename BaseType>
IECore::ConstStringVectorDataPtr ImagePrimitiveSource<BaseType>::computeChannelNames( const Gaffer::Context *context, const ImagePlug *parent ) const
{
	IECore::StringVectorDataPtr result;
	IECore::ConstImagePrimitivePtr image = IECore::staticPointerCast<const IECore::ImagePrimitive>( inputImagePrimitivePlug()->getValue() );
	if( image )
	{
		result = new IECore::StringVectorData;
		image->channelNames( result->writable() );
	}
	return result;
}

template<typename BaseType>
IECore::ConstFloatVectorDataPtr ImagePrimitiveSource<BaseType>::computeChannelData( const std::string &channelName, const Imath::V2i &tileOrigin, const Gaffer::Context *context, const ImagePlug *parent ) const
{
	IECore::ConstImagePrimitivePtr image = IECore::staticPointerCast<const IECore::ImagePrimitive>( inputImagePrimitivePlug()->getValue() );
	if( !image )
	{
		return 0;
	}
	
	IECore::ConstFloatVectorDataPtr channelData = image->getChannel<float>( channelName );
	if( !channelData )
	{
		return 0;
	}
	const std::vector<float> &channel = channelData->readable();
	
	IECore::FloatVectorDataPtr resultData = new IECore::FloatVectorData;
	std::vector<float> &result = resultData->writable();
	result.resize( ImagePlug::tileSize() * ImagePlug::tileSize(), 0.0f );
	
	Imath::Box2i dataWindow = image->getDataWindow();
	Imath::Box2i tileBound( tileOrigin, tileOrigin + Imath::V2i( GafferImage::ImagePlug::tileSize() - 1 ) );
	Imath::Box2i bound = IECore::boxIntersection( tileBound, dataWindow );
	
	for( int y = bound.min.y; y<=bound.max.y; y++ )
	{
		size_t srcIndex = (y - dataWindow.min.y ) * ( dataWindow.size().x + 1 ) + bound.min.x - dataWindow.min.x;
		size_t dstIndex = (y - tileBound.min.y) * GafferImage::ImagePlug::tileSize() + bound.min.x - tileBound.min.x;
		const size_t srcEndIndex = srcIndex + bound.size().x;
		while( srcIndex <= srcEndIndex )
		{
			result[dstIndex++] = channel[srcIndex++];
		}
	}
	
	return resultData;
}

} // namespace GafferImage