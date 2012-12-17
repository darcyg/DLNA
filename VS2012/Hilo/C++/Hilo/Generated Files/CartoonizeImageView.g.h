﻿

#pragma once
//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------

namespace Windows {
    namespace UI {
        namespace Xaml {
            namespace Controls {
                ref class Grid;
                ref class TextBlock;
                ref class Image;
                ref class StackPanel;
                ref class AppBar;
                ref class Button;
            }
        }
    }
}
namespace Windows {
    namespace UI {
        namespace Xaml {
            ref class VisualStateGroup;
            ref class VisualState;
        }
    }
}

namespace Hilo
{
    partial ref class CartoonizeImageView : public ::Hilo::HiloPage, 
        public ::Windows::UI::Xaml::Markup::IComponentConnector
    {
    public:
        void InitializeComponent();
        virtual void Connect(int connectionId, ::Platform::Object^ target);
    
    private:
        bool _contentLoaded;
    
        private: ::Windows::UI::Xaml::Controls::Grid^ ContentRoot;
        private: ::Windows::UI::Xaml::Controls::Grid^ CartoonizeImageGrid;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ PageTitle;
        private: ::Windows::UI::Xaml::Controls::Image^ Photo;
        private: ::Windows::UI::Xaml::Controls::StackPanel^ SlidersStackPanel;
        private: ::Windows::UI::Xaml::VisualStateGroup^ ApplicationViewStates;
        private: ::Windows::UI::Xaml::VisualState^ FullScreenLandscape;
        private: ::Windows::UI::Xaml::VisualState^ Filled;
        private: ::Windows::UI::Xaml::VisualState^ FullScreenPortrait;
        private: ::Windows::UI::Xaml::VisualState^ Snapped;
        private: ::Windows::UI::Xaml::Controls::AppBar^ CartoonizeImageBottomAppBar;
        private: ::Windows::UI::Xaml::Controls::Button^ CancelButton;
        private: ::Windows::UI::Xaml::Controls::Button^ CancelButtonNoLabel;
        private: ::Windows::UI::Xaml::Controls::Button^ SaveButton;
        private: ::Windows::UI::Xaml::Controls::Button^ SaveButtonNoLabel;
        private: ::Windows::UI::Xaml::Controls::Button^ CartoonizeButton;
        private: ::Windows::UI::Xaml::Controls::Button^ ResumeCartoonizeButton;
        private: ::Windows::UI::Xaml::Controls::Button^ CartoonizeButtonNoLabel;
    };
}

