import torch
from googletrans import Translator
from kivy.app import App
from kivy.uix.button import Button
from kivy.uix.gridlayout import GridLayout
from kivy.uix.image import Image
from kivy.uix.label import Label
from kivy.uix.textinput import TextInput
from pytorch_pretrained_bert import GPT2LMHeadModel, GPT2Tokenizer
from torch.nn import functional as F


def Gen_new(orig_input, seq_len=20, l_in='THAI', l_out='EN'):
    m_input = translator.translate(orig_input, dest='en').text
    text = tokenizer.encode(m_input)
    m_input, past = torch.tensor([text]), None
    for _ in range(seq_len):
        logits, past = model(m_input, past=past)
        m_input = torch.multinomial(F.softmax(logits[:, -1], dim=1), 1)
        text.append(m_input.item())
    m_out = tokenizer.decode(text)
    return m_out, translator.translate(m_out, dest='th').text


class SayHello(App):
    def build(self):
        self.window = GridLayout()
        self.window.cols = 1
        self.window.size_hint = (0.6, 0.7)
        self.window.pos_hint = {'center_x': 0.5, 'center_y': 0.5}
        self.m_output0 = Label(text="", font_size=18, color='FFFFFF', halign ='left')
        self.m_output1 = Label(text="", font_size=18, color='FFFFFF', halign ='left')

        # add widgets to window

        # image widget
        self.window.add_widget(Image(source='logo.jpg'))

        # label widget
        self.msg1 = Label(text="\n",font_size=18, color='00FFCE')
        self.window.add_widget(self.msg1)

        # text input widget
        self.user = TextInput(multiline=True, padding_y=(10, 10), size_hint=(1, 1.5))
        self.window.add_widget(self.user)

        # button widget
        self.button = Button(text="Generate !", size_hint=(1, 0.5), bold=True, background_color='32A67F', background_normal='')
        self.button.bind(on_press=self.callback)
        self.window.add_widget(self.button)


        self.window.add_widget(self.m_output0)

        self.window.add_widget(self.m_output1)
        # self.window.add_widget(self.m_output2)

        return self.window

    def callback(self, instance):
        self.m_output1.text, _  = Gen_new(self.user.text, 20)

   


if __name__ == "__main__":
    # Configuration
    tokenizer = GPT2Tokenizer.from_pretrained('gpt2')
    model = GPT2LMHeadModel.from_pretrained('gpt2')
    translator = Translator()

    SayHello().run()
