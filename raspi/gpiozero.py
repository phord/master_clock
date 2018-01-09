# A fake gpiozero for testing on the PC

class LED:
    def __init__(self, pin):
        print("LED init on pin {}".format(pin))

    def on(self):
        pass

    def off(self):
        pass

class Button:
    def __init__(self, pin, pull_up=True):
        print("Button init on pin {} => {}".format(pin, not pull_up))
        self.is_pressed = not pull_up

    is_pressed = False
