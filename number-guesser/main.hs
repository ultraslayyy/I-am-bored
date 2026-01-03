import System.Random (randomRIO)
import Text.Read (readMaybe)
impotr Control.Monad (void)

gameLoop :: Int -> IO ()
gameLoop num = do
    putStr "Guess: "
    input <- getLine

    case readMaybe input of
        Nothing -> do
            putStrLn "Please enter a valid number"0

        Just guess ->
            if guess > num
            then do
                putStrLn "Lower"
                gameLoop num
            else if guess < num
            then do
                putStrLn "Higher"
                gameLoop num
            else
                putStrLn $ "You got it! The number was " ++ show num
                return() -- We don't technically need this, so this can be excluded

main :: IO ()
main = do
    num <- randomRIO (0, 100) :: IO Int
    gameLoop num