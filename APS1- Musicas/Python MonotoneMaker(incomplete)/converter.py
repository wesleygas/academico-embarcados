import mido



mid = mido.MidiFile('megalovania.mid')
table = {
    58: "AS3", 59:"B3",
    60:"C4", 61:"CS4", 62:"D4", 63:"DS4", 64:"E4", 65:"F4", 66:"FS4", 67:"G4", 68:"GS4", 69:"A4", 70:"AS4", 71:"B4",
    72:"C5", 73:"CS5", 74:"D5", 75:"DS5", 76:"E5", 77:"F5", 78:"FS5", 79:"G5", 80:"GS5", 81:"A5", 82:"AS5", 83:"B5",
    84:"C6", 85:"CS6", 86:"D6"
}
notes = []
times = []
final_notes = []
for msg in mid.play():
    if(msg.type == "note_on"):
        if(msg.channel == 0):
            print(msg)
            final_notes.append("NOTE_" + table[msg.note])
    if(msg.type == "note_off"):    
        if(msg.channel == 0):
            print(msg)
            times.append(msg.time)
print(times, len(times), len(final_notes))

