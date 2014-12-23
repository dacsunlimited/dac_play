Feature: Create Dice game, buy game chip and play game DICE
  As a regular user
  I have some fund
  I want to buy some game chip and play game DICE
  So that I might win or lose

  Background: Create an asset and a game binding to that asset
    Given I'm Alice
    And I received 5200000 XTS from angel
	And I wait for one block
	And I created a game asset called DICE with precision 100, initial supply 10000, and inital collateral 1000
	And I wait for one block
    And I created a game called DICE with asset DICE

  Scenario: Buy game chip in the game and then play with it
    When I buy 100 DICE @ 10 DICE/XTS
    And I wait for one block
    Then I should have 90 XTS minus fee
    And I should have 100 DICE
    When I play game Dice with 10 DICE and 50% probability
    Then I have 90 DICE minus fee
    When I wait for one block
    Then I should win 10 DICE or win 0 DICE
