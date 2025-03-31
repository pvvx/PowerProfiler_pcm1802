object FormAdcConfig: TFormAdcConfig
  Left = 554
  Top = 629
  Width = 400
  Height = 194
  Caption = 'ADC Config'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnActivate = FormActivate
  PixelsPerInch = 96
  TextHeight = 13
  object LabelUz: TLabel
    Left = 144
    Top = 60
    Width = 37
    Height = 13
    Caption = 'U offset'
  end
  object LabelIz: TLabel
    Left = 146
    Top = 12
    Width = 32
    Height = 13
    Caption = 'I offset'
  end
  object LabelSPS: TLabel
    Left = 15
    Top = 100
    Width = 26
    Height = 13
    Caption = 'Smps'
  end
  object Label1: TLabel
    Left = 147
    Top = 84
    Width = 32
    Height = 13
    Caption = 'U coef'
  end
  object LabelIk: TLabel
    Left = 151
    Top = 36
    Width = 27
    Height = 13
    Caption = 'I coef'
  end
  object LabelSysClk: TLabel
    Left = 11
    Top = 128
    Width = 49
    Height = 13
    Caption = 'SysCLK: ?'
  end
  object RadioGroupAdcChannels: TRadioGroup
    Left = 8
    Top = 8
    Width = 105
    Height = 81
    Caption = 'ADC Channel'
    ItemIndex = 2
    Items.Strings = (
      'Channel U'
      'Channel I'
      'Channels UI')
    TabOrder = 0
  end
  object ButtonOk: TButton
    Left = 152
    Top = 120
    Width = 75
    Height = 25
    Caption = 'Ok'
    TabOrder = 1
    OnClick = ButtonOkClick
  end
  object ButtonCancel: TButton
    Left = 240
    Top = 120
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object EditUz: TEdit
    Left = 192
    Top = 58
    Width = 89
    Height = 21
    Hint = #1057#1084#1077#1097#1077#1085#1080#1077' '#1076#1083#1103' U'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 3
    Text = '?.?'
  end
  object EditIz: TEdit
    Left = 192
    Top = 10
    Width = 89
    Height = 21
    Hint = #1057#1084#1077#1097#1077#1085#1080#1077' '#1076#1083#1103' U'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 4
    Text = '?.?'
  end
  object ButtonCopyUz: TButton
    Left = 288
    Top = 56
    Width = 89
    Height = 25
    Caption = 'Copy Us to Uz'
    TabOrder = 5
    OnClick = ButtonCopyUzClick
  end
  object ButtonCopyIz: TButton
    Left = 288
    Top = 8
    Width = 89
    Height = 25
    Caption = 'Copy Is to Iz'
    TabOrder = 6
    OnClick = ButtonCopyIzClick
  end
  object SpinEditAdcSmps: TSpinEdit
    Left = 52
    Top = 96
    Width = 69
    Height = 22
    Hint = #1047#1072#1084#1077#1088#1086#1074' '#1074' '#1089#1077#1082' (2000..100000)'
    Increment = 50
    MaxValue = 100000
    MinValue = 1000
    ParentShowHint = False
    ShowHint = True
    TabOrder = 7
    Value = 48000
    OnChange = SpinEditAdcSmpsChange
  end
  object EditUk: TEdit
    Left = 192
    Top = 82
    Width = 89
    Height = 21
    Hint = #1050#1086#1101#1092'. '#1076#1083#1103' U'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 8
    Text = '?.?'
  end
  object EditIk: TEdit
    Left = 192
    Top = 34
    Width = 89
    Height = 21
    Hint = #1050#1086#1101#1092'. '#1076#1083#1103' I'
    ParentShowHint = False
    ShowHint = True
    TabOrder = 9
    Text = '?.?'
  end
end
