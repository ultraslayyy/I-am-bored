module Main exposing (main)

import Browser
import Html exposing (...)
import Html.Events exposing (onInput)
import String

type alias Model =
    { target : Int
    , input : String
    , message : String
    }

init : Model
init =
    { target = 42
    , input = ""
    , message = "Guess: "
    }

update msg model =
    let
        guess =
            String.toInt msg
    in
    case guess of
        Nothing ->
            { model | input = msg, message = "Please enter a valid number" }
        Just g ->
            if g > model.target then
                { model | input = msg, message = "Lower" }
            else if g < model.target then
                { model | input = msg, message = "Higher" }
            else
                { model | input = msg, message = "You got it! The number was " ++ String.fromInt model.target }

view model =
    div []
        [ input [ onInput update ] []
        , div [] [ text model.message ]
        ]

main =
    Browser.sandbox { init = init, update = update, view = view }