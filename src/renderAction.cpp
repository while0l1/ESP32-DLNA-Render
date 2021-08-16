#include "renderAction.h"

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define SOAP_RESPONSE_OUTLINE "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\
<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"\
s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>%s</s:Body></s:Envelope>"

#define SOAP_POSITION_BODY "<u:GetPositionInfoResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\">\
<Track>1</Track>\
<TrackDuration>%s</TrackDuration>\
<TrackMetaData>%s</TrackMetaData>\
<TrackURI>%s</TrackURI>\
<RelTime>%s</RelTime>\
<AbsTime>00:00:00</AbsTime>\
<RelCount>2147483647</RelCount>\
<AbsCount>2147483647</AbsCount>\
</u:GetPositionInfoResponse>"

Audio audio;
String currentURI = "";
String metaData = "";

void playerInit()
{
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(5); // 0...21
}

void playerLoop()
{
    audio.loop();
}

char* playerGetPosition()
{
    char* msg;
    char* body;
    String temp = metaData.substring(metaData.indexOf("duration") + strlen("duration=&quot;"));
    String duration = temp.substring(0, temp.indexOf("&"));
    char relTime[15];
    uint32_t timeInSecond = audio.getAudioCurrentTime();
    int second = timeInSecond % 60;
    int minute = (timeInSecond / 60) % 60;
    int hour = timeInSecond / 3600;
    sprintf(relTime, "%02d:%02d:%02d", hour, minute, second);
    msg = (char*)malloc(sizeof(char) * 1000);
    body = (char*)malloc(sizeof(char) * 800); // 有点浪费空间
    sprintf(body, SOAP_POSITION_BODY, duration.c_str(), "null", "null", relTime);
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    free(body);
    // Serial.println(relTime);
    return msg;
}

char* playerPlay()
{
    audio.pauseResume();
    char* msg;
    msg = (char*)malloc(sizeof(char) * 600);
    char body[] = "<u:PlayResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>";
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    return msg;
}
char* playerPause()
{
    audio.pauseResume();
    char* msg;
    msg = (char*)malloc(sizeof(char) * 600);
    char body[] = "<u:PauseResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>";
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    return msg;
}
char* playerSetAVTransportURI(String xml)
{
    int idx0 = xml.indexOf("<CurrentURI>") + 12;
    int idx1 = xml.indexOf("</CurrentURI>");
    currentURI = xml.substring(idx0, idx1);
    currentURI.replace("&amp;", "&");
    // Serial.printf("URI: %s\n", currentURI.c_str());

    idx0 = xml.indexOf("<CurrentURIMetaData>") + strlen("<CurrentURIMetaData>");
    idx1 = xml.indexOf("</CurrentURIMetaData>");
    metaData = xml.substring(idx0, idx1);
    Serial.printf("MetaData: %s\n", metaData.c_str());

    audio.stopSong();
    audio.connecttohost(currentURI.c_str());
    Serial.printf("FreeHeap: %d\n", ESP.getFreeHeap());
    audio.pauseResume();
    char* msg;
    msg = (char*)malloc(sizeof(char) * 600);
    char body[] = "<u:SetAVTransportURIResponse xmlns:u=\"urn:schemas-upnp-org:service:AVTransport:1\"/>";
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    return msg;
}

int playerSeek(String xml)
{
    int idx0 = xml.indexOf("<Target>") + strlen("<Target>");
    int idx1 = xml.indexOf("</Target>");
    int hour;
    int minute;
    int second;
    sscanf(xml.substring(idx0, idx1).c_str(), "%d:%d:%d", &hour, &minute, &second);
    second = second + minute * 60 + hour * 3600;
    Serial.println(second);
    // free(target);
    // Serial.printf("%d, %d, %d\n", hour, second, minute);
    // audio.setTimeOffset(second);  // 没有效果
    Serial.println("Seek not woring currently");
    return 1;
}
// 手机最大音量是150，尽量跟它匹配吧
// esp32最大音量是21
char* playerGetVolume()
{
    char* msg;
    char* body;
    msg = (char*)malloc(sizeof(char) * 600);
    body = (char*)malloc(sizeof(char) * 200);
    const char bodyTemp[] = "<u:GetVolumeResponse xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"><CurrentVolume>%d</CurrentVolume></u:GetVolumeResponse>";
    sprintf(body, bodyTemp, audio.getVolume() * 5);
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    // Serial.println(msg);
    return msg;
}
char* playerSetVolume(String xml)
{
    int idx0 = xml.indexOf("<DesiredVolume>") + strlen("<DesiredVolume>");
    int idx1 = xml.indexOf("</DesiredVolume>");
    int volume = xml.substring(idx0, idx1).toInt();
    audio.setVolume((uint8_t)volume / 5);
    // Serial.printf("volume: %d", volume);

    char* msg;
    msg = (char*)malloc(sizeof(char) * 600);
    char body[] = "<u:SetVolumeResponse xmlns:u=\"urn:schemas-upnp-org:service:RenderingControl:1\"/>";
    sprintf(msg, SOAP_RESPONSE_OUTLINE, body);
    // Serial.println(msg);
    return msg;
}

int parseActionHeader(String action)
{
    if (action.endsWith("GetPositionInfo\""))
        return CODE_GetPositionInfo;
    else if (action.endsWith("Play\""))
        return CODE_Play;
    else if (action.endsWith("Pause\""))
        return CODE_Pause;
    else if (action.endsWith("SetAVTransportURI\""))
        return CODE_SetAVTransportURI;
    else if (action.endsWith("Seek\""))
        return CODE_Seek;
    else if (action.endsWith("GetVolume\""))
        return CODE_GetVolume;
    else if (action.endsWith("SetVolume\""))
        return CODE_SetVolume;
    else
        return -1;
}

// optional
void audio_info(const char* info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char* info)
{ //id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char* info)
{ //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
// void audio_showstation(const char* info)
// {
//     Serial.print("station     ");
//     Serial.println(info);
// }
void audio_showstreamtitle(const char* info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char* info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char* info)
{ //duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
// void audio_icyurl(const char* info)
// { //homepage
//     Serial.print("icyurl      ");
//     Serial.println(info);
// }
void audio_lasthost(const char* info)
{ //stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
// void audio_eof_speech(const char* info)
// {
//     Serial.print("eof_speech  ");
//     Serial.println(info);
// }