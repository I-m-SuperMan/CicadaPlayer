package com.cicada.player.utils.media;

import android.media.DeniedByServerException;
import android.media.MediaDrm;
import android.media.NotProvisionedException;
import android.media.ResourceBusyException;
import android.media.UnsupportedSchemeException;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.util.Base64;

import com.cicada.player.utils.Logger;
import com.cicada.player.utils.NativeUsed;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

import static android.media.MediaDrm.EVENT_KEY_REQUIRED;
import static android.media.MediaDrm.EVENT_PROVISION_REQUIRED;

@NativeUsed
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class DrmSessionManager {

    private static final String TAG = DrmSessionManager.class.getSimpleName();

    private static final String WIDEVINE_FORMAT = "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
    public static final UUID WIDEVINE_UUID = new UUID(0xEDEF8BA979D64ACEL, 0xA3C827DCD51D21EDL);

    private long mNativeInstance = 0;

    public DrmSessionManager(long nativeInstance) {
        mNativeInstance = nativeInstance;
    }

    private List<DrmSession> drmSessionList = new ArrayList<>();

    public static int SESSION_STATE_ERROR = -1;
    public static int SESSION_STATE_IDLE = -2;
    public static int SESSION_STATE_OPENED = 0;

    public static int ERROR_CODE_NONE = 0;
    public static int ERROR_CODE_UNSUPPORT_SCHEME = 1;
    public static int ERROR_CODE_RESOURCE_BUSY = 2;
    public static int ERROR_CODE_KEY_RESPONSE_NULL = 3;
    public static int ERROR_CODE_PROVISION_RESPONSE_NULL = 4;
    public static int ERROR_CODE_DENIED_BY_SERVER = 5;
    public static int ERROR_CODE_RELEASED = 6;

    private static class DrmInfo {
        public String licenseUrl = null;
        public String keyUrl = null;
        public String keyFormat = null;
        public String mime = null;

        public boolean isSame(DrmInfo info) {
            if (info == null) {
                return false;
            }

            if (!areEqual(keyUrl, info.keyUrl)) {
                return false;
            }

            if (!areEqual(licenseUrl, info.licenseUrl)) {
                return false;
            }

            if (!areEqual(keyFormat, info.keyFormat)) {
                return false;
            }
            return true;
        }

        private static boolean areEqual(Object o1, Object o2) {
            return o1 == null ? o2 == null : o1.equals(o2);
        }
    }

    private class DrmSession {
        public DrmInfo drmInfo = null;
        public MediaDrm mediaDrm = null;
        public byte[] sessionId = null;

        public int referenceCount = 0;
        public int state = SESSION_STATE_IDLE;

        private HandlerThread requestHandlerThread = null;
        public Handler requestHandler = null;

        public DrmSession(DrmInfo info) {
            drmInfo = info;

            requestHandlerThread = new HandlerThread("DrmRequestHanderThread");
            requestHandlerThread.start();
            requestHandler = new Handler(requestHandlerThread.getLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    int event = msg.what;
                    Logger.v(TAG, "  handleMessage event = " + event);
                    if (event == EVENT_PROVISION_REQUIRED) {
                        requestProvision();
                    } else if (event == EVENT_KEY_REQUIRED
                            || event == MediaDrm.EVENT_KEY_EXPIRED) {
                        try {
                            requestKey();
                        } catch (NotProvisionedException e) {
                            requestProvision();
                        }
                    }

                    super.handleMessage(msg);
                }
            };
        }

        public boolean prepare() {
//            try {
                try {
                    if (WIDEVINE_FORMAT.equals(drmInfo.keyFormat)) {
                        mediaDrm = new MediaDrm(WIDEVINE_UUID);
                    } else {
                        Logger.e(TAG, " prepare fail : not support format :" + drmInfo.keyFormat);
                        changeState(SESSION_STATE_ERROR , ERROR_CODE_UNSUPPORT_SCHEME);
                        return false;
                    }
                } catch (UnsupportedSchemeException e) {
                    Logger.e(TAG, " prepare fail : " + e.getMessage());
                    changeState(SESSION_STATE_ERROR , ERROR_CODE_UNSUPPORT_SCHEME);
                    return false;
                }

                mediaDrm.setOnEventListener(new MediaDrm.OnEventListener() {
                    @Override
                    public void onEvent(@NonNull MediaDrm md, @Nullable byte[] sessionId, int event, int extra, @Nullable byte[] data) {
                        Logger.d(TAG, " drm Event = " + event + " , extra = " + extra);
                        sendRequest(event, sessionId);
                    }
                });

                try {
                    sessionId = mediaDrm.openSession();
                    changeState(SESSION_STATE_IDLE , ERROR_CODE_NONE);
                    sendRequest(EVENT_KEY_REQUIRED, sessionId);
                } catch (NotProvisionedException e) {
                    sendRequest(EVENT_PROVISION_REQUIRED, null);
                } catch (ResourceBusyException e) {
                    Logger.e(TAG, " prepare fail : " + e.getMessage());
                    changeState(SESSION_STATE_ERROR , ERROR_CODE_RESOURCE_BUSY);
                    return false;
                }

            return true;
        }

        private void sendRequest(int event, byte[] sessionId) {
            Message msg = requestHandler.obtainMessage(event, sessionId);
            requestHandler.sendMessage(msg);
        }

        public boolean release() {
            referenceCount--;
            if (referenceCount > 0) {
                return false;
            }

            changeState(SESSION_STATE_ERROR,ERROR_CODE_RELEASED);

            requestHandlerThread.quit();

            if (mediaDrm != null) {
                mediaDrm.closeSession(sessionId);
                mediaDrm.release();
                mediaDrm = null;
            }

            return true;
        }

        private void requestKey() throws NotProvisionedException {
            Logger.v(TAG, "requestKey ");
            try {
                byte[] initData = Base64.decode(drmInfo.keyUrl.substring(drmInfo.keyUrl.indexOf(',')), Base64.DEFAULT);
                MediaDrm.KeyRequest keyRequest = mediaDrm.getKeyRequest(sessionId, initData,
                        drmInfo.mime, MediaDrm.KEY_TYPE_STREAMING, null);
                byte[] requestData = native_requestKey(mNativeInstance, keyRequest.getDefaultUrl(), keyRequest.getData());

                Logger.v(TAG, "requestKey result = " + requestData);

                if (requestData == null) {
                    Logger.e(TAG, "requestKey fail: data = null , url : " + keyRequest.getDefaultUrl());
                    changeState( SESSION_STATE_ERROR , ERROR_CODE_KEY_RESPONSE_NULL);
                    return;
                }

                mediaDrm.provideKeyResponse(sessionId, requestData);
                changeState( SESSION_STATE_OPENED,ERROR_CODE_NONE);
            } catch (DeniedByServerException e) {
                Logger.e(TAG, "requestKey fail: " + e.getMessage());
                changeState( SESSION_STATE_ERROR, ERROR_CODE_DENIED_BY_SERVER);
            }
        }

        private void requestProvision() {
            Logger.v(TAG, "requestProvision ");
            MediaDrm.ProvisionRequest request = mediaDrm.getProvisionRequest();
            byte[] provisionData = native_requestProvision(mNativeInstance, request.getDefaultUrl(), request.getData());
            if (provisionData == null) {
                Logger.e(TAG, "requestProvision fail: data = null , url : " + request.getDefaultUrl());
                changeState( SESSION_STATE_ERROR , ERROR_CODE_PROVISION_RESPONSE_NULL);
                return;
            }

            try {
                mediaDrm.provideProvisionResponse(provisionData);
                sendRequest(EVENT_KEY_REQUIRED, sessionId);
            } catch (DeniedByServerException e) {
                Logger.e(TAG, "requestProvision fail: " + e.getMessage());
                changeState(SESSION_STATE_ERROR , ERROR_CODE_DENIED_BY_SERVER);
                return;
            }

        }

        public byte[] requireSessionId() {
            referenceCount++;
            return sessionId;
        }

        private void changeState(int state , int errorCode ) {
            this.state = state;
            Logger.d(TAG, "changeState " + state);
            if(sessionId != null) {
                native_changeState(mNativeInstance, sessionId, state, errorCode);
            }
        }
    }

    public synchronized byte[] requireSession(String keyUrl, String keyFormat, String mime, String licenseUrl) {
        Logger.d(TAG, "requireSessionInner info = " + keyFormat);

        DrmInfo info = new DrmInfo();
        info.licenseUrl = licenseUrl;
        info.keyFormat = keyFormat;
        info.keyUrl = keyUrl;
        info.mime = mime;

        return requireSessionInner(info);
    }

    private byte[] requireSessionInner(DrmInfo info) {
        for (DrmSession drmSession : drmSessionList) {
            if (drmSession.drmInfo.isSame(info)) {
                return drmSession.requireSessionId();
            }
        }

        DrmSession drmSession = new DrmSession(info);
        boolean successed = drmSession.prepare();
        if (successed) {
            drmSessionList.add(drmSession);
            return drmSession.requireSessionId();
        }

        return null;
    }

    public synchronized void releaseSession(byte[] sessionId) {
        Logger.d(TAG, "releaseSession sessionId = " + sessionId);
        DrmSession session = getDrmSession(sessionId);
        if (session != null) {
            session.release();
            drmSessionList.remove(session);
        }
    }

    private DrmSession getDrmSession(byte[] sessionId) {
        for (DrmSession drmSession : drmSessionList) {
            if (Arrays.equals(drmSession.sessionId, sessionId)) {
                return drmSession;
            }
        }
        return null;
    }

    protected native byte[] native_requestProvision(long nativeInstance, String url, byte[] data);

    protected native byte[] native_requestKey(long nativeInstance, String url, byte[] data);

    protected native void native_changeState(long nativeInstance, byte[] sessionId, int state, int errorCode );
}
