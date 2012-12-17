#ifndef __DJTaskGroup_h__
#define __DJTaskGroup_h__

#include "DJUtils.h"

namespace deejay {

class TaskThread;
class TaskGroup;

class Task
{
public:
	virtual ~Task();
	bool aborted() const;
	void abort();

protected:
	Task();
	virtual void exec() = 0;
	virtual void doAbort();
	TaskGroup *taskGroup() const;

private:
	friend class TaskThread;
	friend class TaskGroup;

	bool m_abortFlag;
	TaskGroup *m_taskGroup;
};

class TaskThread
	: public NPT_Thread
{
public:
	TaskThread(TaskGroup *pool);
	virtual ~TaskThread();
	virtual void Run();
	void abort();
	NPT_Result start(Task *task);
	void setTask(Task *task);

private:
	void internalSetTask(Task *task);
	void wakeUp();
	NPT_Result timedWait(NPT_Timeout timeout);

private:
	TaskGroup *m_pool;
	Task *m_task;
	Task *m_ic;
	Task *m_sc;
	NPT_SharedVariable m_var;
	ReadWriteLock m_stateLock;
	bool m_abortFlag;
};

class TaskGroup
{
public:
	TaskGroup();
	~TaskGroup();

	enum State {
		State_Running,
		State_Stopping,
		State_Stopped
	};

	NPT_Result startTask(Task *task);
	void reset();
	void abort();
	NPT_Result wait(NPT_Timeout timeout = NPT_TIMEOUT_INFINITE);

private:
	NPT_Result startNewThread(Task *task);
	void joinThreads();

private:
	friend class TaskThread;

	State m_state;

	ReadWriteLock m_freeThreadListLock;
	NPT_List<TaskThread*> m_freeThreadList;

	ReadWriteLock m_threadListLock;
	NPT_List<TaskThread*> m_threadList;

	ReadWriteLock m_stateLock;
};

} // namespace deejay

#endif // __DJTaskGroup_h__
