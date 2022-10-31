#pragma once
#include <memory>
#include <utility>

namespace PolyM {

/** Type for Msg unique identifiers */
using MsgUID = unsigned long long;

/* 灯光装置命令字 ID */
#define    POLYM_FOCAL_SETTING          1 /* 调焦命令字 */
#define    POLYM_GREEN_MOCODE_SETTING   2 /* 绿光莫码控制 */
#define    POLYM_GREEN_BLINK_SETTING    3 /* 绿光频闪控制 */
#define    POLYM_GREEN_NORMAL_SETTING   4 /* 绿光开关控制 */
#define    POLYM_WHITE_MOCODE_SETTING   5 /* 白光莫码控制 */
#define    POLYM_WHITE_BLINK_SETTING    6 /* 白光频闪控制 */
#define    POLYM_WHITE_NORMAL_SETTING   7 /* 白光开关控制 */

#define    POLYM_TURNTABLE_LEFT_SETTING   8 /* 左转控制 */
#define    POLYM_TURNTABLE_RIGHT_SETTING  9 /* 右转控制 */
#define    POLYM_TURNTABLE_DOWN_SETTING   10 /* 向下控制 */
#define    POLYM_TURNTABLE_UP_SETTING     11 /* 向上控制 */
#define    POLYM_TURNTABLE_STOP_SETTING   12 /* 停止 */

#define    POLYM_TURNTABLE_MODE_SETTING   13 /* 模式设置 */

/* JSON 文件配置 ID */
#define    POLYM_BUTTON_CONFIRM  2
#define    POLYM_BUTTON_CANCEL   1

/**
 * Msg represents a simple message that doesn't have any payload data.
 * Msg ID identifies the type of the message. Msg ID can be queried with getMsgId().
 */
class Msg
{
public:
    /**
     * Construct a Msg.
     *
     * @param msgId Msg ID of this Msg.
     */
    Msg(int msgId);

    virtual ~Msg() = default;
    Msg(const Msg&) = delete;
    Msg& operator=(const Msg&) = delete;

    /** "Virtual move constructor" */
    virtual std::unique_ptr<Msg> move();

    /**
     * Get Msg ID.
     * Msg ID identifies message type.
     * Multiple Msg instances can have the same Msg ID.
     */
    int getMsgId() const;

    /**
     * Get Msg UID.
     * Msg UID is the unique ID associated with this message.
     * All Msg instances have a unique Msg UID.
     */
    MsgUID getUniqueId() const;

protected:
    Msg(Msg&&) = default;
    Msg& operator=(Msg&&) = default;

private:
    int msgId_;
    MsgUID uniqueId_;
};

/**
 * DataMsg<PayloadType> is a Msg with payload of type PayloadType.
 * Payload is constructed when DataMsg is created and the DataMsg instance owns the payload data.
 */
template <typename PayloadType>
class DataMsg : public Msg
{
public:
    /**
     * Construct DataMsg
     * @param msgId Msg ID
     * @param args Arguments for PayloadType ctor
     */
    template <typename ... Args>
    DataMsg(int msgId, Args&& ... args)
      : Msg(msgId),
        pl_(new PayloadType(std::forward<Args>(args) ...))
    {
    }

    virtual ~DataMsg() = default;
    DataMsg(const DataMsg&) = delete;
    DataMsg& operator=(const DataMsg&) = delete;

    /** "Virtual move constructor" */
    virtual std::unique_ptr<Msg> move() override
    {
        return std::unique_ptr<Msg>(new DataMsg<PayloadType>(std::move(*this)));
    }

    /** Get the payload data */
    PayloadType& getPayload() const
    {
        return *pl_;
    }

protected:
    DataMsg(DataMsg&&) = default;
    DataMsg& operator=(DataMsg&&) = default;

private:
    std::unique_ptr<PayloadType> pl_;
};

}
