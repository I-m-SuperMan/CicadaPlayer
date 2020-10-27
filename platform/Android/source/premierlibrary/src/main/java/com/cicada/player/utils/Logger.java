package com.cicada.player.utils;

import android.util.Log;

public class Logger {

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }

    /**
     * 日志等级
     */
    /****
     * Log level
     */
    public enum LogLevel {

        /**
         * 无
         */
        /****
         * None
         */
        AF_LOG_LEVEL_NONE(0),
        /**
         * 崩溃
         */
        /****
         * Fatal
         */
        AF_LOG_LEVEL_FATAL(8),
        /**
         * 错误
         */
        /****
         * Error
         */
        AF_LOG_LEVEL_ERROR(16),
        /**
         * 警告
         */
        /****
         * Warning
         */
        AF_LOG_LEVEL_WARNING(24),
        /**
         * 信息
         */
        /****
         * Info
         */
        AF_LOG_LEVEL_INFO(32),
        /**
         * 调试
         */
        /****
         * Debug
         */
        AF_LOG_LEVEL_DEBUG(48),
        /**
         * 堆栈
         */
        /****
         * Trace
         */
        AF_LOG_LEVEL_TRACE(56);

        private int mValue;

        private LogLevel(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }

    }

    /**
     * 日志回调接口
     */
    /****
     * Log callback.
     */
    public interface OnLogCallback {
        /**
         *
         * @param level 日志等级。 见{@linkplain LogLevel}
         * @param msg 日志信息
         */
        /****
         *
         * @param level The level of the log. See {@linkplain LogLevel}.
         * @param msg The log data.
         */
        public void onLog(LogLevel level, String msg);
    }

    private static OnLogCallback mLogCallback = null;

    /**
     * 设置日志回调
     * @param callback 回调。
     */
    /****
     * Set a log callback.
     * @param callback The log callback.
     */
    public static void setLogCallback(OnLogCallback callback) {
        mLogCallback = callback;
    }

    public static OnLogCallback getLogCallback() {
        return mLogCallback;
    }

    private static boolean sConsoleLogEnabled  = true;

    /**
     * 设置日志回调
     * @param logLevel 日志等级。 见{@linkplain LogLevel}。
     */
    /****
     * Set a log callback.
     * @param logLevel The level of the log. See {@linkplain LogLevel}.
     */
    public static void setLogLevel(LogLevel logLevel) {
        nSetLogLevel(logLevel.getValue());
    }

    public static int GetLogLevel() {
        return nGetLogLevel();
    }

    public static void enableConsoleLog(boolean bEnabled) {
        sConsoleLogEnabled = bEnabled;
        nEnableConsoleLog(bEnabled);
    }

    private static native void nSetLogLevel(int level);

    private static native int nGetLogLevel();

    private static native void nEnableConsoleLog(boolean enable);

    private static void nOnLogCallback(int level, byte[] msg) {
        if (mLogCallback != null) {
            String msgStr = new String(msg);
            mLogCallback.onLog(getLevel(level), msgStr);
        }
    }

    private static LogLevel getLevel(int level) {
        switch (level) {
            case 0:
                return LogLevel.AF_LOG_LEVEL_NONE;
            case 8:
                return LogLevel.AF_LOG_LEVEL_FATAL;
            case 16:
                return LogLevel.AF_LOG_LEVEL_ERROR;
            case 24:
                return LogLevel.AF_LOG_LEVEL_WARNING;
            case 32:
                return LogLevel.AF_LOG_LEVEL_INFO;
            case 48:
                return LogLevel.AF_LOG_LEVEL_DEBUG;
            case 56:
                return LogLevel.AF_LOG_LEVEL_TRACE;
            default:
                return LogLevel.AF_LOG_LEVEL_DEBUG;
        }
    }

    public static void e(String tag, String msg){
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_ERROR.getValue() ){
            Log.e(tag , msg);
        }
    }

    public static void w(String tag, String msg){
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_WARNING.getValue() ){
            Log.w(tag , msg);
        }
    }

    public static void i(String tag, String msg){
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_INFO.getValue() ){
            Log.i(tag , msg);
        }
    }

    public static void d(String tag, String msg){
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_DEBUG.getValue() ){
            Log.d(tag , msg);
        }
    }

    public static void v(String tag, String msg){
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_TRACE.getValue() ){
            Log.v(tag , msg);
        }
    }

    public static void t() {
        if(sConsoleLogEnabled && GetLogLevel() >= LogLevel.AF_LOG_LEVEL_TRACE.getValue() ){
            Thread thread = Thread.currentThread(); // 获取当前线程
            StackTraceElement[] trace = thread.getStackTrace(); // 获取当前线程的栈快照(入栈方法的数据)
            Log.v(trace[3].getFileName() , trace[3].getMethodName() + ":" + trace[3].getLineNumber());
        }
    }

}
