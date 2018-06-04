
#include "universal_timer.h"



#define MAX_NUM_TIMERS   10


/******************************************************************************
* 文件名称：SoftTimer.c
* 内容摘要：软件定时器模块
* 其他说明：首先运行TimersInit函数，需向该函数提供"1ms的系统时钟"和"最大系统ms数"，
*         然后在各自的应用模块中调用CreatTimer创建定时器，该函数返回的地址为该定
*         时器的地址，可用与重启或删除定时器结点，请妥善保管。请在主循环中执行
*         ProcessTimer函数以更新定时器时间。
* 当前版本：V1.00
* 作 者：  David Han, Ian
* 完成日期：2015年2月20日
******************************************************************************/

static TIMER_TABLE* sg_ptTimeTableHead = NULL;             /* 链表表头       */
static TMRSOURCE    sg_pfSysClk        = NULL;             /* 系统1ms时钟函数 */
static long int     sg_dwTimeMaxValue  = MAX_VALUE_16_BIT; /* 最大ms数       */


static  TIMER_TABLE timerList[MAX_NUM_TIMERS];
static TIMER_TABLE  timerHead;


/*************************************************************************
* 函数名称：int timer_init(TMRSOURCE pfTimer, u32 dwMaxTime)
* 功能说明：初始化软件定时器模块
* 输入参数：TMRSOURCE pfTimer  系统1ms时钟函数
           u32   dwMaxTime 时钟函数最大ms数
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
           SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int timer_init(TMRSOURCE pfTimer, u32 dwMaxTime)
{
    if (NULL == pfTimer)
    {
        return SW_ERROR; /* 检查注册函数是否为空指针 */
    }
    
    sg_ptTimeTableHead = &timerHead; /* 申请头结点 */
    if (NULL == sg_ptTimeTableHead)
    { 
        return SW_ERROR; /* 检查是否申请成功 */
    }

    /* 申请成功后进行初始化 */
    sg_ptTimeTableHead->next = NULL;               /* 下个结点地址置空     */
    sg_pfSysClk              = (TMRSOURCE)pfTimer; /* 注册系统1ms时钟函数  */
    sg_dwTimeMaxValue        = dwMaxTime;          /* 确定时钟函数最大ms数 */
    
    return SW_OK;
}

/*************************************************************************
* 函数名称：TIMER_TABLE* creat_timer(u32 dwTimeout, u8 ucPeriodic, TMRCALLBACK pfTimerCallback, void *pArg)
* 功能说明：创建并启动软件定时器
* 输入参数：u32       dwTimeout  0~0xFFFFFFFF 定时时间
            u8       ucPeriodic  SINGLE      单次触发
                                   PERIODIC    周期触发
           TMRCALLBACK pfTimerCallback         定时结束时回调函数
           void       *pArg                    回调函数参数
            
* 输出参数：无
* 返 回 值：操作失败 : NULL
           操作成功 : 定时器模块指针
* 其它说明：创建完定时器后返回定时器结点的地址，改地址用于重启或删除该定时器
**************************************************************************/
TIMER_TABLE* creat_timer(u32 dwTimeout, u8 ucPeriodic, TMRCALLBACK pfTimerCallback, void *pArg)
{
    TIMER_TABLE* ptTimerNode;
    TIMER_TABLE* ptFind;
	static u8 timerIndex = 0;
    if (NULL == sg_ptTimeTableHead)
    {
        return NULL; /* 检查链表头节点是否存在 */
    }

    /* 链表头结点已经存在 */
  
    if (timerIndex >= MAX_NUM_TIMERS)
    {
        return NULL; /* 检查是否申请成功 */
    }
	ptTimerNode = &timerList[timerIndex]; /* 申请定时器结点 */
	timerIndex++;

    /* 结点申请成功 */
    ptTimerNode->next                 = NULL;                    /* 下个结点地址置空 */
	ptTimerNode->timerData.timeStat        = TIMER_STOP;
    ptTimerNode->timerData.periodic        = ucPeriodic;              /* 单次/周期触发 */
    ptTimerNode->timerData.start           = sg_pfSysClk();           /* 获取计时起始时间 */
    ptTimerNode->timerData.now             = ptTimerNode->timerData.start; /* 获取当前时间 */
    ptTimerNode->timerData.elapse          = 0;                       /* 已经过的时间 */
    ptTimerNode->timerData.timeout         = dwTimeout;               /* 定时时间 */
    ptTimerNode->timerData.pfTimerCallback = pfTimerCallback;         /* 注册定时结束回调函数 */
    ptTimerNode->timerData.pArg            = pArg;                    /* 回调函数参数 */

    /* 将新申请的定时器结点增加进入链表 */
    ptFind = sg_ptTimeTableHead; /* 先找链表头结点 */
    while(NULL != ptFind->next)  /* 如果当前结点不是末尾结点*/
    {
        ptFind = ptFind->next;   /* 将下一个结点的地址作为当前结点继续查找 */
    }
    /* 找到末尾结点 */
    ptFind->next= ptTimerNode;   /* 将新申请结点链接到末尾结点 */

    return ptTimerNode;          /* 操作成功，返回新申请结点地址(用于删除和重启计时) */
}

/*************************************************************************
* 函数名称：int stop_timer(TIMER_TABLE* ptNode)
* 功能说明：停止定时器结点
* 输入参数：TIMER_TABLE* ptNode 定时器结点地址
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
           SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int stop_timer(TIMER_TABLE* ptNode)
{

    if (ptNode)
    {
		ptNode->timerData.timeStat = TIMER_STOP;
    }
	else
    {
        return SW_ERROR; /* 检查定时器结点是否为空 */
    }
    return SW_OK;                    
}

/*************************************************************************
* 函数名称：int start_timer(TIMER_TABLE* ptNode)
* 功能说明：停止定时器结点
* 输入参数：TIMER_TABLE* ptNode 定时器结点地址
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
           SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int start_timer(TIMER_TABLE* ptNode)
{

    if (ptNode)
    {
		ptNode->timerData.start           = sg_pfSysClk();           /* 获取计时起始时间 */
    	ptNode->timerData.now             = ptNode->timerData.start; /* 获取当前时间 */
		ptNode->timerData.timeStat = TIMER_RUNNING;
    }
	else
    {
        return SW_ERROR; /* 检查定时器结点是否为空 */
    }
    return SW_OK;                    
}

/*************************************************************************
* 函数名称：int set_timer_time(TIMER_TABLE* ptNode)
* 功能说明：设置定时器定时间
* 输入参数：TIMER_TABLE* ptNode 定时器结点地址
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
           SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int set_timer_time(TIMER_TABLE* ptNode, u32 ticks)
{

    if (ptNode)
    {
		ptNode->timerData.timeout  = ticks;         
    }
	else
    {
        return SW_ERROR; /* 检查定时器结点是否为空 */
    }
    return SW_OK;                    
}


/*************************************************************************
* 函数名称：int reset_timer(TIMER_TABLE* ptNode)
* 功能说明：重启定时器结点
* 输入参数：TIMER_TABLE* ptNode 定时器结点地址
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
           SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int reset_timer(TIMER_TABLE* ptNode)
{
    if (NULL == ptNode)
    {
        return SW_ERROR;                /* 检查定时器结点是否为空 */
    }

    ptNode->timerData.start = sg_pfSysClk(); /* 更新定时器起始时间 */
    return SW_OK;                       /* 操作成功 */
}


/*************************************************************************
* 函数名称：int process_timer(void)
* 功能说明：更新定时器结点
* 输入参数：无
* 输出参数：无
* 返 回 值：SW_ERROR: 操作失败
            SW_OK 操作成功
* 其它说明：无
**************************************************************************/
int process_timer(void)
{
    TIMER_TABLE* ptFind;
	
    if (NULL == sg_ptTimeTableHead)
    { 
        return SW_ERROR; /* 检查是否申请成功 */
    }
    ptFind = sg_ptTimeTableHead->next;    /* 找到第一个有效结点 */
    while(ptFind)                         /* 如果不是末尾结点 */
    { 
		if(TIMER_RUNNING == ptFind->timerData.timeStat)
		{
        	ptFind->timerData.now = sg_pfSysClk(); /* 更新时间 */

	        /* 计算此刻时间与起始时间的时间差 */
	        if(ptFind->timerData.now >= ptFind->timerData.start)
	        {
	            ptFind->timerData.elapse = ptFind->timerData.now - ptFind->timerData.start;
	        }
	        else
	        {
	            ptFind->timerData.elapse = sg_dwTimeMaxValue - ptFind->timerData.start + ptFind->timerData.now;
	        }
	        
	        if(ptFind->timerData.elapse >= ptFind->timerData.timeout)          /* 如果时差大于等于设定的计时时间 */
	        {
	            
	            if(ptFind->timerData.periodic)
	            {
	                reset_timer(ptFind);                              /* 如果是周期性触发，重启定时器 */
	            }
	            else
	            {                                                    /* 如果是单次触发，删除定时器 */ 
	                stop_timer(ptFind); 
	            }  
				if(ptFind->timerData.pfTimerCallback)                     /* 且已经注册了合法的回调函数 */
	            {
	                ptFind->timerData.pfTimerCallback(ptFind->timerData.pArg); /* 执行回调函数 */
	            }    
	        }
		}
       	ptFind = ptFind->next;                                   /* 继续更新下一个定时器结点 */
		
    }
    return SW_OK;                                                /* 操作成功 */
}


/* end of file */





