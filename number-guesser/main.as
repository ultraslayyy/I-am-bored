package {
    import flash.display.Sprite;
    import flash.events.Event;
    import flash.text.TextField;
    import flash.text.TextFieldType;

    public class Main extends Sprite {
        private var num:int;
        private var num:TextField;
        private var output:TextField;

        public function Main() {
            num = Math.floor(Math.random() * 101);

            output = new TextField();
            output.width = 400;
            output.height = 300;
            addChild(output);

            input = new TextField();
            input.type = TextFieldType.INPUT;
            input.border = true;
            input.y = 320;
            input.width = 200;
            addChild(input);

            input.addEventListener(Event.CHANGE, function(e:Event):void {
                var guess:int = int(input.text)

                if (input.text == "" || isNaN(guess)) {
                    output.text = "Please enter a valid number";
                    return;
                }

                if (guess > num) {
                    output.text = "Lower";
                } else if (guess < num) {
                    output.text = "Higher";
                } else {
                    output.text = "You got it! The number was " + num;
                }
            });
        }
    }
}