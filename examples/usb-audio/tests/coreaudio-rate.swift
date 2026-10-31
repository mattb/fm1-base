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
guard CommandLine.arguments.count == 3, let rate = Double(CommandLine.arguments[2]), [44100.0,48000.0].contains(rate) else { fatalError("usage: coreaudio-rate EXACT_DEVICE_NAME 44100|48000") }
let name = CommandLine.arguments[1]
let devices = ids(AudioObjectID(kAudioObjectSystemObject), kAudioHardwarePropertyDevices).filter { string($0,kAudioObjectPropertyName) == name }
guard devices.count == 1 else { fatalError("require exactly one matching device") }
var a = address(kAudioDevicePropertyNominalSampleRate)
var value = rate
let status = AudioObjectSetPropertyData(devices[0], &a, 0, nil, UInt32(MemoryLayout<Double>.size), &value)
guard status == noErr else { fatalError("set rate failed: \(status)") }
print("Requested \(rate) Hz for \(name); verify physical format and USB GET_CUR during capture")
