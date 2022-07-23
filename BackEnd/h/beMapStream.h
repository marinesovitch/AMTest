// author: marines marinesovitch alias Darek Slusarczyk 2012-2013, 2022
#ifndef INC_BE_MAP_STREAM_H
#define INC_BE_MAP_STREAM_H

namespace be
{

struct IMapStream
{
	public:
		static IMapStream* create( const std::string& fname );
		static IMapStream* create( const int fileDescriptor, const long offset );
		static IMapStream* create( const int* begin, const int* end );
		static IMapStream* create();

	public:
		virtual ~IMapStream() = default;

	public:
		virtual bool getInt( int* value ) = 0;

};

} // namespace be

#endif
