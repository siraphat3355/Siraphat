//
//  ContentView.swift
//  BMI
//
//  Created by Siraphat B on 16/7/2564 BE.
//

import SwiftUI

struct ContentView: View {
    @State var resultText:String = "BMI Calculator"
    @State var weight:String = ""
    @State var height:String = ""
    var body: some View {
        VStack {
            Text(resultText)
                .padding()
            
            HStack {
                Text("Weight ")
                TextField("Kilogram", text: $weight).keyboardType(.numberPad)
            }.padding()
            
            HStack {
                Text("Height")
                TextField("Centimetres", text: $height).keyboardType(.numberPad)
            }.padding()
            
            Button(action: {
                let result = Double(self.weight)! / pow((Double(self.height)! / 100.0), 2.0)
                self.resultText = String(result)
                
            }) {
                Text("Start Calculate")
            }.padding()
        }.padding()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
