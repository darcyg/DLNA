#include "DJTaskGroup.h"

/*----------------------------------------------------------------------
|   logging
+---------------------------------------------------------------------*/
NPT_SET_LOCAL_LOGGER("deejay.TaskGroup")

namespace deejay {

Task::Task()
	: m_abortFlag(false), m_taskGroup(NULL)
{
}

Task::~Task()
{
}

TaskGroup *Task::taskGroup() const
{
	return m_taskGroup;
}

bool Task::aborted() const
{
	return m_abortFlag;
}

void Task::abort()
{
	if (!m_abortFlag) {
		m_abortFlag = true;
		doAbort();
	}
}

void Task::doAbort()
{
}

TaskThread::TaskThread(TaskGroup *pool)
	: m_pool(pool), m_task(NULL)
	, m_abortFlag(false), m_ic(NULL), m_sc(NULL)
{
	m_var.SetValue(0);
}

TaskThread::~TaskThread()
{
}

void TaskThread::wakeUp()
{
	m_var.SetValue(1);
}

NPT_Result TaskThread::timedWait(NPT_Timeout timeout)
{
	NPT_Result nr = m_var.WaitUntilEquals(1, timeout);
	m_var.SetValue(0);
	return nr;
}

void TaskThread::internalSetTask(Task *task)
{
	WriteLocker locker(m_stateLock);
	m_task = task;
	if (m_abortFlag && m_task) {
		m_task->abort();
	}
}

void TaskThread::Run()
{
	NPT_LOG_FINEST_2("TaskThread %p [tid=%d] in", static_cast<NPT_Thread*>(this), GetCurrentThreadId());
	Task *task = m_ic;
	if (task) {
		internalSetTask(task);
		task->exec();
		internalSetTask(NULL);
		delete task;
	}

	while (!m_abortFlag) {
		{
			WriteLocker locker(m_pool->m_freeThreadListLock);
			m_sc = NULL;
			m_pool->m_freeThreadList.Add(this);
		}

		NPT_Result nr = timedWait(120 * 1000);

		task = NULL;

		{
			WriteLocker locker(m_pool->m_freeThreadListLock);
			TaskThread *th = this;
			NPT_List<TaskThread*>::Iterator it = m_pool->m_freeThreadList.Find(NPT_ObjectComparator<TaskThread*>(th));
			if (it) {
				m_pool->m_freeThreadList.Erase(it);
			}

			task = m_sc;
		}

		if (task) {
			internalSetTask(task);
			task->exec();
			internalSetTask(NULL);
			delete task;
		}

		if (nr == NPT_ERROR_TIMEOUT && !task) {
			break;
		}
	}
	NPT_LOG_FINEST_2("TaskThread %p [tid=%d] out", static_cast<NPT_Thread*>(this), GetCurrentThreadId());
}

void TaskThread::abort()
{
	NPT_LOG_FINEST_2("TaskThread::abort %p [tid=%d] in", static_cast<NPT_Thread*>(this), GetCurrentThreadId());
	WriteLocker locker(m_stateLock);
	if (!m_abortFlag) {
		NPT_LOG_FINEST_2("TaskThread::abort %p [tid=%d] 1st", static_cast<NPT_Thread*>(this), GetCurrentThreadId());
		m_abortFlag = true;
		if (m_task) {
			NPT_LOG_FINEST_2("TaskThread::abort %p [tid=%d] task abort", static_cast<NPT_Thread*>(this), GetCurrentThreadId());
			m_task->abort();
		}
		wakeUp();
	}
}

NPT_Result TaskThread::start(Task *task)
{
	m_ic = task;
	NPT_Result nr = NPT_Thread::Start();
	if (NPT_FAILED(nr)) {
		delete task;
		m_ic = NULL;
	}
	return nr;
}

void TaskThread::setTask(Task *task)
{
	m_sc = task;
	wakeUp();
}

TaskGroup::TaskGroup()
	: m_state(State_Running)
{
}

TaskGroup::~TaskGroup()
{
	wait();
}

NPT_Result TaskGroup::startTask(Task *task)
{
	WriteLocker locker(m_stateLock);

	if (m_state != State_Running) {
		delete task;
		return NPT_ERROR_INVALID_STATE;
	}

	joinThreads();

	task->m_taskGroup = this;

	{
		TaskThread *thread = NULL;
		WriteLocker locker(m_freeThreadListLock);
		NPT_List<TaskThread*>::Iterator it = m_freeThreadList.GetFirstItem();
		if (it) {
			thread = *it;
			m_freeThreadList.Erase(it);
			thread->setTask(task);
			return NPT_SUCCESS;
		}
	}

	return startNewThread(task);
}

void TaskGroup::reset()
{
	WriteLocker locker(m_stateLock);
	if (m_state == State_Stopped) {
		m_state = State_Running;
	}
}

void TaskGroup::abort()
{
	WriteLocker locker(m_stateLock);
	if (m_state == State_Running) {
		m_state = State_Stopping;
		for (NPT_Ordinal i = 0; i < m_threadList.GetItemCount(); i++) {
			TaskThread *thread = *m_threadList.GetItem(i);
			thread->abort();
		}
	}
}

NPT_Result TaskGroup::wait(NPT_Timeout timeout)
{
	NPT_Result nr;
	{
		WriteLocker locker(m_stateLock);
		if (m_state != State_Stopping) {
			return NPT_ERROR_INVALID_STATE;
		}
	}

	if (timeout == NPT_TIMEOUT_INFINITE) {
		/*for (NPT_Ordinal i = 0; i < m_threadList.GetItemCount(); i++) {
			TaskThread *thread = *m_threadList.GetItem(i);
			thread->Wait();
			delete thread;
		}
		m_threadList.Clear();*/
		NPT_LOG_INFO_2("TaskGroup %p waiting for %d threads", this, m_threadList.GetItemCount());
		while (m_threadList.GetItemCount() > 0) {
			NPT_Cardinal count = m_threadList.GetItemCount();
			for (NPT_Ordinal i = 0; i < count; ) {
				NPT_List<TaskThread*>::Iterator it = m_threadList.GetItem(i);
				TaskThread *thread = *it;
				NPT_LOG_INFO_2("TaskGroup %p waiting for thread %p", this, thread);
				if (NPT_SUCCEEDED(thread->Wait(3000))) {
					NPT_LOG_INFO_2("TaskGroup %p waited thread %p, fine!", this, thread);
					delete thread;
					m_threadList.Erase(it);
					count--;
				} else {
					NPT_LOG_INFO_2("TaskGroup %p thread %p still pending, continue", this, thread);
					i++;
				}
			}
		}
	} else {
		NPT_Timeout waitTick = 10;
		if (waitTick > timeout) {
			waitTick = timeout;
		}

		NPT_TimeStamp ts1, ts2;
		NPT_Timeout totalTick = 0;

		while (m_threadList.GetItemCount() != 0 && timeout >= totalTick) {

			NPT_Cardinal count = m_threadList.GetItemCount();
			NPT_Ordinal i = 0;
			while (i < count) {
				NPT_List<TaskThread*>::Iterator it = m_threadList.GetItem(i);
				TaskThread *thread = *it;
				NPT_System::GetCurrentTimeStamp(ts1);
				nr = thread->Wait(waitTick);
				NPT_System::GetCurrentTimeStamp(ts2);
				totalTick += (ts2 - ts1).ToMillis();

				if (NPT_SUCCEEDED(nr)) {
					m_threadList.Erase(it);
					delete thread;
					--count;
				} else {
					i++;
				}

				if (timeout >= totalTick) {
					NPT_Timeout remainTick = timeout - totalTick;
					if (waitTick > remainTick) {
						waitTick = remainTick;
					}
				} else {
					waitTick = 0;
				}
			}
		}
	}

	if (m_threadList.GetItemCount() == 0) {
		m_state = State_Stopped;
	}

	return m_threadList.GetItemCount() == 0 ? NPT_SUCCESS : NPT_ERROR_TIMEOUT;
}

NPT_Result TaskGroup::startNewThread(Task *task)
{
	TaskThread *thread = new TaskThread(this);
	WriteLocker locker(m_threadListLock);
	m_threadList.Add(thread);
	NPT_Result nr = thread->start(task);
	if (NPT_FAILED(nr)) {
		m_threadList.Remove(thread);
		delete thread;
		delete task;
	}
	return nr;
}

void TaskGroup::joinThreads()
{
	NPT_Cardinal count = m_threadList.GetItemCount();
	NPT_Ordinal i = 0;
	while (i < count) {
		NPT_List<TaskThread*>::Iterator it = m_threadList.GetItem(i);
		TaskThread *thread = *it;
		if (NPT_SUCCEEDED(thread->Wait(0))) {
			m_threadList.Erase(it);
			delete thread;
			--count;
		} else {
			i++;
		}
	}
}

} // namespace deejay
