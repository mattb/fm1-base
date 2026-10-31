import Foundation
import CoreAudio

func address(_ selector: AudioObjectPropertySelector, _ scope: AudioObjectPropertyScope = kAudioObjectPropertyScopeGlobal, _ element: AudioObjectPropertyElement = kAudioObjectPropertyElementMain) -> AudioObjectPropertyAddress {
    AudioObjectPropertyAddress(mSelector: selector, mScope: scope, mElement: element)
}
func string(_ object: AudioObjectID, _ selector: AudioObjectPropertySelector, _ scope: AudioObjectPropertyScope = kAudioObjectPropertyScopeGlobal, _ element: AudioObjectPropertyElement = kAudioObjectPropertyElementMain) -> String? {
    var a = address(selector, scope, element); var value: Unmanaged<CFString>?; var size = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
    guard AudioObjectGetPropertyData(object, &a, 0, nil, &size, &value) == noErr else { return nil }
    return value?.takeRetainedValue() as String?
}
func ids(_ object: AudioObjectID, _ selector: AudioObjectPropertySelector, _ scope: AudioObjectPropertyScope = kAudioObjectPropertyScopeGlobal) -> [AudioObjectID] {
    var a = address(selector, scope); var size: UInt32 = 0
    guard AudioObjectGetPropertyDataSize(object, &a, 0, nil, &size) == noErr else { return [] }
    var result = [AudioObjectID](repeating: 0, count: Int(size) / MemoryLayout<AudioObjectID>.size)
    guard AudioObjectGetPropertyData(object, &a, 0, nil, &size, &result) == noErr else { return [] }
    return result
}
func format(_ stream: AudioObjectID, _ selector: AudioObjectPropertySelector) -> [String:Any]? {
    var a = address(selector); var f = AudioStreamBasicDescription(); var size = UInt32(MemoryLayout<AudioStreamBasicDescription>.size)
    guard AudioObjectGetPropertyData(stream, &a, 0, nil, &size, &f) == noErr else { return nil }
    return ["rate":f.mSampleRate,"channels":f.mChannelsPerFrame,"bits":f.mBitsPerChannel,"bytes_per_frame":f.mBytesPerFrame,"format_id":f.mFormatID,"flags":f.mFormatFlags]
}
let filter = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : ""
var output = [[String:Any]]()
for device in ids(AudioObjectID(kAudioObjectSystemObject), kAudioHardwarePropertyDevices) {
    let name = string(device, kAudioObjectPropertyName) ?? ""
    if !filter.isEmpty && !name.localizedCaseInsensitiveContains(filter) { continue }
    let streams = ids(device,kAudioDevicePropertyStreams,kAudioDevicePropertyScopeInput)
    var list = [[String:Any]](); var channels = 0
    for stream in streams {
        let physical = format(stream,kAudioStreamPropertyPhysicalFormat) ?? [:]
        channels += Int(physical["channels"] as? UInt32 ?? 0)
        list.append(["id":stream,"name":string(stream,kAudioObjectPropertyName) ?? NSNull(),"physical":physical,"virtual":format(stream,kAudioStreamPropertyVirtualFormat) ?? [:]])
    }
    var names = [Any]()
    if channels > 0 { for ch in 1...channels { names.append(string(device,kAudioObjectPropertyElementName,kAudioDevicePropertyScopeInput,UInt32(ch)) ?? NSNull()) } }
    output.append(["id":device,"name":name,"uid":string(device,kAudioDevicePropertyDeviceUID) ?? NSNull(),"input_channels":channels,"channel_names":names,"input_streams":list])
}
let data = try JSONSerialization.data(withJSONObject:output,options:[.prettyPrinted,.sortedKeys])
FileHandle.standardOutput.write(data); print("")
