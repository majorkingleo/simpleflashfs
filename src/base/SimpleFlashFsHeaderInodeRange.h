#pragma once

namespace SimpleFlashFs::base {

template <class Config>
class HeaderInodeRangeInterface
{
public:
	virtual ~HeaderInodeRangeInterface() = default;

	virtual void	 reset() = 0;
	virtual bool 	 has_next() const = 0;
	virtual uint32_t next() = 0;

	// reset and then call next() to get the first value
	virtual uint32_t start() = 0;
};

} // namespace SimpleFlashFs::base