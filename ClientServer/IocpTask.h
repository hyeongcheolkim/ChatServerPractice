#pragma once
class IocpTask
{
public:
	virtual HANDLE getHandle() abstract;
	virtual void dispatch() abstract;
};

