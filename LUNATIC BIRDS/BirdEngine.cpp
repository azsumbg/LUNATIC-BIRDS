#include "pch.h"
#include "BirdEngine.h"

dll::BASIC_FIELD* dll::CreateFieldItem(fields what_field, float where_x, float where_y)
{
	BASIC_FIELD* ret = nullptr;
	ret = new BASIC_FIELD(what_field, where_x, where_y);
	return ret;
}

dll::BASIC_PIG* dll::CreatePig(float first_x, float first_y, pigs what)
{
	BASIC_PIG* ret = nullptr;
	ret = new BASIC_PIG(what, first_x, first_y);
	return ret;
}

dll::BASIC_BIRD* dll::CreateBird(float first_x, float first_y, birds what)
{
	Bird ret = nullptr;
	ret = new BASIC_BIRD(what, first_x, first_y);
	return ret;
}