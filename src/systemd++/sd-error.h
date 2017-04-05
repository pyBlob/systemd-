// Copyright 2017-2017 Karl Kraus. See LICENSE for legal info

#pragma once

namespace sd
{

class error
{
public:
	const int code;

	error(int code) : code(code) {}
};

}; // namespace sd

